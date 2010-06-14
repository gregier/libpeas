/*
 * peas-plugin-loader-python.c
 * This file is part of libpeas
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
 * Copyright (C) 2009 - Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-extension-python.h"
#include "peas-plugin-loader-python.h"

/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * glib-object.h, so we unset it here to avoid a warning. Yep, that's bad. */
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <pygobject.h>
#include <signal.h>

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

struct _PeasPluginLoaderPythonPrivate {
  GHashTable *loaded_plugins;
  guint idle_gc;
  guint init_failed : 1;
  guint must_finalize_python : 1;
  PyThreadState *py_thread_state;
};

typedef struct {
  PyObject *module;
} PythonInfo;

static PyObject *PyGObject_Type;

static gboolean   peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *self,
                                                             const gchar            *module_path);

G_DEFINE_DYNAMIC_TYPE (PeasPluginLoaderPython, peas_plugin_loader_python, PEAS_TYPE_PLUGIN_LOADER);

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_plugin_loader_python_register_type (G_TYPE_MODULE (module));
  peas_extension_python_register (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_PYTHON);
}

/* NOTE: This must be called with the GIL held */
static PyTypeObject *
find_python_extension_type (PeasPluginInfo *info,
                            GType           exten_type,
                            PyObject       *pymodule)
{
  PyObject *pygtype, *pytype;
  PyObject *locals, *key, *value;
  Py_ssize_t pos = 0;

  locals = PyModule_GetDict (pymodule);

  pygtype = pyg_type_wrapper_new (exten_type);
  pytype = PyObject_GetAttrString (pygtype, "pytype");
  g_return_val_if_fail (pytype != NULL, NULL);

  if (pytype == Py_None)
    return NULL;

  while (PyDict_Next (locals, &pos, &key, &value))
    {
      if (!PyType_Check (value))
        continue;

      switch (PyObject_IsSubclass (value, pytype))
        {
        case 1:
          Py_DECREF (pygtype);
          return (PyTypeObject *) value;
        case 0:
          continue;
        case -1:
          PyErr_Print ();
          continue;
        }
    }

  Py_DECREF (pygtype);
  g_debug ("No %s derivative found in Python plugin '%s'",
           g_type_name (exten_type), peas_plugin_info_get_name (info));
  return NULL;
}

static gboolean
peas_plugin_loader_python_provides_extension (PeasPluginLoader *loader,
                                              PeasPluginInfo   *info,
                                              GType             exten_type)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PythonInfo *pyinfo;
  PyTypeObject *extension_type;
  PyGILState_STATE state;

  pyinfo = (PythonInfo *) g_hash_table_lookup (pyloader->priv->loaded_plugins, info);

  if (pyinfo == NULL)
    return FALSE;

  state = pyg_gil_state_ensure ();
  extension_type = find_python_extension_type (info, exten_type, pyinfo->module);
  pyg_gil_state_release (state);

  return extension_type != NULL;
}

static PeasExtension *
peas_plugin_loader_python_get_extension (PeasPluginLoader *loader,
                                         PeasPluginInfo   *info,
                                         GType             exten_type)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PythonInfo *pyinfo;
  PyTypeObject *pytype;
  PyObject *pyobject;
  PyGObject *pygobject;
  PyObject *emptyarg;
  PyObject *pyplinfo;
  PyGILState_STATE state;
  PeasExtension *exten;

  pyinfo = (PythonInfo *) g_hash_table_lookup (pyloader->priv->loaded_plugins, info);

  if (pyinfo == NULL)
    return NULL;

  state = pyg_gil_state_ensure ();

  pytype = find_python_extension_type (info, exten_type, pyinfo->module);

  if (pytype == NULL || pytype->tp_new == NULL)
    {
      pyg_gil_state_release (state);
      return NULL;
    }

  emptyarg = PyTuple_New (0);
  pyobject = pytype->tp_new (pytype, emptyarg, NULL);
  Py_DECREF (emptyarg);

  if (pyobject == NULL)
    {
      pyg_gil_state_release (state);
      g_error ("Could not create instance for %s.",
               peas_plugin_info_get_name (info));

      return NULL;
    }

  pygobject = (PyGObject *) pyobject;

  if (pygobject->obj != NULL)
    {
      Py_DECREF (pyobject);
      pyg_gil_state_release (state);
      g_error ("Could not create instance for %s (GObject already initialized).",
               peas_plugin_info_get_name (info));

      return NULL;
    }

  pygobject_construct (pygobject, NULL);

  if (pygobject->obj == NULL)
    {
      Py_DECREF (pyobject);
      pyg_gil_state_release (state);
      g_error ("Could not create %s instance for %s (GObject not constructed).",
               g_type_name (exten_type), peas_plugin_info_get_name (info));

      return NULL;
    }

  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (pygobject->obj, exten_type), NULL);

  /* now call tp_init manually */
  if (PyType_IsSubtype (pyobject->ob_type, pytype) &&
      pyobject->ob_type->tp_init != NULL)
    {
      emptyarg = PyTuple_New (0);
      pyobject->ob_type->tp_init (pyobject, emptyarg, NULL);
      Py_DECREF (emptyarg);
    }

  /* Set the plugin info as an attribute of the instance */
  pyplinfo = pyg_boxed_new (PEAS_TYPE_PLUGIN_INFO, info, TRUE, TRUE);
  PyObject_SetAttrString (pyobject, "plugin_info", pyplinfo);
  Py_DECREF (pyplinfo);

  exten = peas_extension_python_new (exten_type, pyobject);
  pyg_gil_state_release (state);

  return exten;
}

/* NOTE: This must be called with the GIL held */
static void
add_python_info (PeasPluginLoaderPython *loader,
                 PeasPluginInfo         *info,
                 PyObject               *module)
{
  PythonInfo *pyinfo;

  pyinfo = g_new (PythonInfo, 1);
  pyinfo->module = module;
  Py_INCREF (pyinfo->module);

  g_hash_table_insert (loader->priv->loaded_plugins, info, pyinfo);
}

static void
peas_plugin_loader_python_add_module_directory (PeasPluginLoader *loader,
                                                const gchar      *module_dir)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PyGILState_STATE state = pyg_gil_state_ensure ();

  g_debug ("Adding %s as a module path for the python loader.", module_dir);
  peas_plugin_loader_python_add_module_path (pyloader, module_dir);

  pyg_gil_state_release (state);
}

static gboolean
peas_plugin_loader_python_load (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PyObject *pymodule, *fromlist;
  gchar *module_name;
  PyGILState_STATE state;

  if (pyloader->priv->init_failed)
    {
      g_warning ("Cannot load python plugin Python '%s' since libpeas was "
                 "not able to initialize the Python interpreter.",
                 peas_plugin_info_get_name (info));

      return FALSE;
    }

  /* see if py definition for the plugin is already loaded */
  if (g_hash_table_lookup (pyloader->priv->loaded_plugins, info))
    return TRUE;

  state = pyg_gil_state_ensure ();

  /* If we have a special path, we register it */
  peas_plugin_loader_python_add_module_path (pyloader,
                                             peas_plugin_info_get_module_dir (info));

  /* we need a fromlist to be able to import modules with a '.' in the
     name. */
  fromlist = PyTuple_New (0);
  module_name = g_strdup (peas_plugin_info_get_module_name (info));

  pymodule = PyImport_ImportModuleEx (module_name, NULL, NULL, fromlist);

  Py_DECREF (fromlist);

  if (!pymodule)
    {
      g_free (module_name);
      PyErr_Print ();
      pyg_gil_state_release (state);

      return FALSE;
    }

  add_python_info (pyloader, info, pymodule);

  pyg_gil_state_release (state);

  return TRUE;
}

static void
peas_plugin_loader_python_unload (PeasPluginLoader *loader,
                                  PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PythonInfo *pyinfo;
  PyGILState_STATE state;

  pyinfo = (PythonInfo *) g_hash_table_lookup (pyloader->priv->loaded_plugins, info);

  if (!pyinfo)
    return;

  state = pyg_gil_state_ensure ();
  Py_XDECREF (pyinfo->module);
  pyg_gil_state_release (state);

  pyinfo->module = NULL;
}

static gboolean
run_gc_protected (void)
{
  PyGILState_STATE state = pyg_gil_state_ensure ();

  while (PyGC_Collect ())
    ;

  pyg_gil_state_release (state);
}

static gboolean
run_gc (PeasPluginLoaderPython *loader)
{
  run_gc_protected ();

  loader->priv->idle_gc = 0;
  return FALSE;
}

static void
peas_plugin_loader_python_garbage_collect (PeasPluginLoader *loader)
{
  PeasPluginLoaderPython *pyloader;

  if (!Py_IsInitialized ())
    return;

  pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);

  /*
   * We both run the GC right now and we schedule
   * a further collection in the main loop.
   */
  run_gc_protected ();

  if (pyloader->priv->idle_gc == 0)
    pyloader->priv->idle_gc = g_idle_add ((GSourceFunc) run_gc, pyloader);
}

static void
peas_python_shutdown (PeasPluginLoaderPython *loader)
{
  if (!Py_IsInitialized ())
    return;

  if (loader->priv->py_thread_state)
    {
      PyEval_RestoreThread (loader->priv->py_thread_state);
      loader->priv->py_thread_state = NULL;
    }

  if (loader->priv->idle_gc != 0)
    {
      g_source_remove (loader->priv->idle_gc);
      loader->priv->idle_gc = 0;
    }

  run_gc_protected ();

  if (loader->priv->must_finalize_python)
    {
      pyg_gil_state_ensure ();
      Py_Finalize ();
    }
}

/* C equivalent of
 *    import sys
 *    sys.path.insert(0, module_path)
 */
/* NOTE: This must be called with the GIL held */
static gboolean
peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *self,
                                           const gchar            *module_path)
{
  PyObject *pathlist, *pathstring;

  g_return_val_if_fail (PEAS_IS_PLUGIN_LOADER_PYTHON (self), FALSE);
  g_return_val_if_fail (module_path != NULL, FALSE);

  if (!Py_IsInitialized ())
    return FALSE;

  pathlist = PySys_GetObject ("path");
  pathstring = PyString_FromString (module_path);

  if (PySequence_Contains (pathlist, pathstring) == 0)
    PyList_Insert (pathlist, 0, pathstring);
  Py_DECREF (pathstring);

  return TRUE;
}

/* Note: the following function is needed because init_pyobject is a *macro*
 * which in case of error set the PyErr and then make the calling function
 * return behind our back.
 * It's up to the caller to check the result with PyErr_Occurred ()
 */
static void
peas_init_pygobject (void)
{
  init_pygobject_check (2, 11, 5);      /* FIXME: get from config */
}

static gboolean
peas_python_init (PeasPluginLoaderPython *loader)
{
  PyObject *mdict, *gobject, *gi, *gettext, *install, *gettext_args;
  char *argv[] = { "", NULL };
  gchar *prgname;

  /* We are trying to initialize Python for the first time,
     set init_failed to FALSE only if the entire initialization process
     ends with success */
  loader->priv->init_failed = TRUE;

  /* Python initialization */
  if (!Py_IsInitialized ())
    {
      Py_InitializeEx (FALSE);
      loader->priv->must_finalize_python = TRUE;
    }

  prgname = g_get_prgname ();
  if (prgname != NULL)
      argv[0] = prgname;

  PySys_SetArgv (1, argv);

  /* Note that we don't call this with the GIL held, since we haven't initialised pygobject yet */
  peas_plugin_loader_python_add_module_path (loader, PEAS_PYEXECDIR);

  /* import gobject */
  peas_init_pygobject ();
  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import pygobject.");

      goto python_init_error;
    }

  /* Initialize support for threads */
  pyg_enable_threads ();

  gobject = PyImport_ImportModule ("gobject");
  if (gobject == NULL)
    {
      g_warning ("Error initializing Python interpreter: cound not "
                 "import gobject.");

      goto python_init_error;
    }

  mdict = PyModule_GetDict (gobject);
  PyGObject_Type = PyDict_GetItemString (mdict, "GObject");
  if (PyGObject_Type == NULL)
    {
      g_warning ("Error initializing Python interpreter: cound not "
                 "get gobject.GObject");

      goto python_init_error;
    }

  gi = PyImport_ImportModule ("gi");
  if (gi == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import gi.");

      goto python_init_error;
    }

  /* i18n support */
  gettext = PyImport_ImportModule ("gettext");
  if (gettext == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import gettext.");

      goto python_init_error;
    }

  mdict = PyModule_GetDict (gettext);
  install = PyDict_GetItemString (mdict, "install");
  gettext_args = Py_BuildValue ("ss", GETTEXT_PACKAGE, PEAS_LOCALEDIR);
  PyObject_CallObject (install, gettext_args);
  Py_DECREF (gettext_args);

  /* Python has been successfully initialized */
  loader->priv->init_failed = FALSE;

  loader->priv->py_thread_state = PyEval_SaveThread ();

  return TRUE;

python_init_error:

  g_warning ("Please check the installation of all the Python related packages "
             "required by libpeas and try again.");

  PyErr_Clear ();

  peas_python_shutdown (loader);

  return FALSE;
}

static void
destroy_python_info (PythonInfo *info)
{
  PyGILState_STATE state = pyg_gil_state_ensure ();
  Py_XDECREF (info->module);
  pyg_gil_state_release (state);

  g_free (info);
}

static void
peas_plugin_loader_python_init (PeasPluginLoaderPython *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            PEAS_TYPE_PLUGIN_LOADER_PYTHON,
                                            PeasPluginLoaderPythonPrivate);

  /* initialize python interpreter */
  peas_python_init (self);

  /* loaded_plugins maps EggPluginsInfo to a PythonInfo */
  self->priv->loaded_plugins = g_hash_table_new_full (g_direct_hash,
                                                      g_direct_equal,
                                                      NULL,
                                                      (GDestroyNotify) destroy_python_info);
}

static void
peas_plugin_loader_python_finalize (GObject *object)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (object);

  g_hash_table_destroy (pyloader->priv->loaded_plugins);
  peas_python_shutdown (pyloader);

  G_OBJECT_CLASS (peas_plugin_loader_python_parent_class)->finalize (object);
}

static void
peas_plugin_loader_python_class_init (PeasPluginLoaderPythonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  object_class->finalize = peas_plugin_loader_python_finalize;

  loader_class->add_module_directory = peas_plugin_loader_python_add_module_directory;
  loader_class->load = peas_plugin_loader_python_load;
  loader_class->unload = peas_plugin_loader_python_unload;
  loader_class->get_extension = peas_plugin_loader_python_get_extension;
  loader_class->provides_extension = peas_plugin_loader_python_provides_extension;
  loader_class->garbage_collect = peas_plugin_loader_python_garbage_collect;

  g_type_class_add_private (object_class,
                            sizeof (PeasPluginLoaderPythonPrivate));
}

static void
peas_plugin_loader_python_class_finalize (PeasPluginLoaderPythonClass *klass)
{
}
