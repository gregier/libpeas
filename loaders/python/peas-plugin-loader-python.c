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

#include "peas-extension-python.h"
#include "peas-plugin-loader-python.h"

#include <Python.h>
#include <pygobject.h>
#include <signal.h>
#include "config.h"

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

struct _PeasPluginLoaderPythonPrivate {
  GHashTable *loaded_plugins;
  guint idle_gc;
  gboolean init_failed;
};

typedef struct {
  PyObject *module;
} PythonInfo;

static PyObject *PyGObject_Type;

static gboolean   peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *self,
                                                             const gchar            *module_path);

G_DEFINE_DYNAMIC_TYPE (PeasPluginLoaderPython, peas_plugin_loader_python, PEAS_TYPE_PLUGIN_LOADER);

static GObject *
create_object (GType the_type)
{
  return g_object_new (the_type, NULL);
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_plugin_loader_python_register_type (G_TYPE_MODULE (module));
  peas_extension_python_register (G_TYPE_MODULE (module));

  peas_object_module_register_extension (module,
                                         PEAS_TYPE_PLUGIN_LOADER,
                                         (PeasCreateFunc) create_object,
                                         GSIZE_TO_POINTER (PEAS_TYPE_PLUGIN_LOADER_PYTHON));
}

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

  while (PyDict_Next (locals, &pos, &key, &value))
    {
      if (!PyType_Check (value))
        continue;

      if (PyObject_IsSubclass (value, pytype))
        {
          Py_DECREF (pygtype);
          return (PyTypeObject *) value;
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

  pyinfo = (PythonInfo *) g_hash_table_lookup (pyloader->priv->loaded_plugins, info);

  if (pyinfo == NULL)
    return FALSE;

  return find_python_extension_type (info, exten_type, pyinfo->module) != NULL;
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

  pyinfo = (PythonInfo *) g_hash_table_lookup (pyloader->priv->loaded_plugins, info);

  if (pyinfo == NULL)
    return NULL;

  pytype = find_python_extension_type (info, exten_type, pyinfo->module);

  if (pytype == NULL || pytype->tp_new == NULL)
    return NULL;

  emptyarg = PyTuple_New (0);
  pyobject = pytype->tp_new (pytype, emptyarg, NULL);
  Py_DECREF (emptyarg);

  if (pyobject == NULL)
    {
      g_error ("Could not create instance for %s.",
               peas_plugin_info_get_name (info));
      return NULL;
    }

  pygobject = (PyGObject *) pyobject;

  if (pygobject->obj != NULL)
    {
      Py_DECREF (pyobject);
      g_error
        ("Could not create instance for %s (GObject already initialized).",
         peas_plugin_info_get_name (info));
      return NULL;
    }

  pygobject_construct (pygobject, NULL);

  if (pygobject->obj == NULL)
    {
      g_error ("Could not create %s instance for %s (GObject not constructed).",
               g_type_name (exten_type), peas_plugin_info_get_name (info));
      Py_DECREF (pyobject);

      return NULL;
    }

  /* now call tp_init manually */
  if (PyType_IsSubtype (pyobject->ob_type, pytype)
      && pyobject->ob_type->tp_init != NULL)
    {
      emptyarg = PyTuple_New (0);
      pyobject->ob_type->tp_init (pyobject, emptyarg, NULL);
      Py_DECREF (emptyarg);
    }

  /* Set the plugin info as an attribute of the instance */
  pyplinfo = pyg_boxed_new (PEAS_TYPE_PLUGIN_INFO, info, TRUE, TRUE);
  PyObject_SetAttrString (pyobject, "plugin_info", pyplinfo);
  Py_DECREF (pyplinfo);

  return peas_extension_python_new (exten_type, pyobject);
}

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

  g_debug ("Adding %s as a module path for the python loader.", module_dir);
  peas_plugin_loader_python_add_module_path (pyloader, module_dir);
}

static gboolean
peas_plugin_loader_python_load (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PyObject *pymodule, *fromlist;
  gchar *module_name;

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
      return FALSE;
    }

  add_python_info (pyloader, info, pymodule);

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
run_gc (PeasPluginLoaderPython *loader)
{
  while (PyGC_Collect ())
    ;

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

  while (PyGC_Collect ())
    ;

  if (pyloader->priv->idle_gc == 0)
    pyloader->priv->idle_gc = g_idle_add ((GSourceFunc) run_gc, pyloader);
}

static void
peas_python_shutdown (PeasPluginLoaderPython *loader)
{
  if (!Py_IsInitialized ())
    return;

  if (loader->priv->idle_gc != 0)
    {
      g_source_remove (loader->priv->idle_gc);
      loader->priv->idle_gc = 0;
    }

  while (PyGC_Collect ())
    ;

  Py_Finalize ();
}

/* C equivalent of
 *    import sys
 *    sys.path.insert(0, module_path)
 */
static gboolean
peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *self,
                                           const gchar            *module_path)
{
  PyObject *pathlist, *pathstring;

  g_return_val_if_fail (PEAS_IS_PLUGIN_LOADER_PYTHON (self), FALSE);
  g_return_val_if_fail (module_path != NULL, FALSE);

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
  PyObject *mdict, *gobject, *gettext, *install, *gettext_args;
  char *argv[] = { "libpeas", NULL };
#ifdef HAVE_SIGACTION
  struct sigaction old_sigint;
  gint res;
#endif

  if (loader->priv->init_failed)
    {
      /* We already failed to initialized Python, don't need to
       * retry again */
      return FALSE;
    }

  if (Py_IsInitialized ())
    {
      /* Python has already been successfully initialized */
      return TRUE;
    }

  /* We are trying to initialize Python for the first time,
     set init_failed to FALSE only if the entire initialization process
     ends with success */
  loader->priv->init_failed = TRUE;

  /* Hack to make python not overwrite SIGINT: this is needed to avoid
   * the crash reported on bug #326191 */

  /* CHECK: can't we use Py_InitializeEx instead of Py_Initialize in order
     to avoid to manage signal handlers ? - Paolo (Dec. 31, 2006) */

#ifdef HAVE_SIGACTION
  /* Save old handler */
  res = sigaction (SIGINT, NULL, &old_sigint);
  if (res != 0)
    {
      g_warning ("Error initializing Python interpreter: cannot get "
                 "handler to SIGINT signal (%s)", g_strerror (errno));

      return FALSE;
    }
#endif

  /* Python initialization */
  Py_Initialize ();

#ifdef HAVE_SIGACTION
  /* Restore old handler */
  res = sigaction (SIGINT, &old_sigint, NULL);
  if (res != 0)
    {
      g_warning ("Error initializing Python interpreter: cannot restore "
                 "handler to SIGINT signal (%s).", g_strerror (errno));

      goto python_init_error;
    }
#endif

  PySys_SetArgv (1, argv);

  peas_plugin_loader_python_add_module_path (loader, PEAS_PYEXECDIR);

  /* import gobject */
  peas_init_pygobject ();
  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import pygobject.");

      goto python_init_error;
    }

  gobject = PyImport_ImportModule ("gobject");
  if (gobject == NULL)
    {
      g_warning ("Error initializing Python interpreter: cound not "
                 "import gobject.");
      goto python_init_error;
    }

  mdict = PyModule_GetDict (gobject);
  PyGObject_Type = PyDict_GetItemString (mdict, "GObject");
  if (!PyGObject_Type)
    {
      g_warning ("Error initializing Python interpreter: cound not "
                 "get gobject.GObject");
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
