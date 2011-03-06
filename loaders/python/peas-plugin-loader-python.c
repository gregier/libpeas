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

static gboolean   peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *pyloader,
                                                             const gchar            *module_path);

G_DEFINE_TYPE (PeasPluginLoaderPython, peas_plugin_loader_python, PEAS_TYPE_PLUGIN_LOADER);

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
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

  if (pytype != Py_None)
    {
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
            default:
              PyErr_Print ();
              continue;
            }
        }
    }

  Py_DECREF (pygtype);

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

  state = pyg_gil_state_ensure ();
  extension_type = find_python_extension_type (info, exten_type, pyinfo->module);
  pyg_gil_state_release (state);

  return extension_type != NULL;
}

static PeasExtension *
peas_plugin_loader_python_create_extension (PeasPluginLoader *loader,
                                            PeasPluginInfo   *info,
                                            GType             exten_type,
                                            guint             n_parameters,
                                            GParameter       *parameters)
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
      g_warning ("Could not create instance for '%s'",
                 peas_plugin_info_get_name (info));

      return NULL;
    }

  pygobject = (PyGObject *) pyobject;

  if (pygobject->obj != NULL)
    {
      Py_DECREF (pyobject);
      pyg_gil_state_release (state);
      g_warning ("Could not create instance for '%s': GObject already initialized",
                 peas_plugin_info_get_name (info));

      return NULL;
    }

  pygobject_constructv (pygobject, n_parameters, parameters);

  if (pygobject->obj == NULL)
    {
      Py_DECREF (pyobject);
      pyg_gil_state_release (state);
      g_warning ("Could not create '%s' instance for '%s': GObject not constructed",
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
  Py_DECREF (pyobject);

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

static gboolean
peas_plugin_loader_python_load (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PyObject *pymodule, *fromlist;
  const gchar *module_name;
  PyGILState_STATE state;

  if (pyloader->priv->init_failed)
    {
      g_warning ("Cannot load Python plugin '%s' since libpeas was "
                 "not able to initialize the Python interpreter",
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
  module_name = peas_plugin_info_get_module_name (info);

  pymodule = PyImport_ImportModuleEx ((gchar *) module_name, NULL, NULL, fromlist);

  Py_DECREF (fromlist);

  if (!pymodule)
    {
      PyErr_Print ();
      pyg_gil_state_release (state);

      return FALSE;
    }

  add_python_info (pyloader, info, pymodule);

  Py_DECREF (pymodule);

  pyg_gil_state_release (state);

  return TRUE;
}

static void
peas_plugin_loader_python_unload (PeasPluginLoader *loader,
                                  PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);

  g_hash_table_remove (pyloader->priv->loaded_plugins, info);
}

static void
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
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);

  if (pyloader->priv->init_failed)
    return;

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
peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *pyloader,
                                           const gchar            *module_path)
{
  PyObject *pathlist, *pathstring;

  g_return_val_if_fail (PEAS_IS_PLUGIN_LOADER_PYTHON (pyloader), FALSE);
  g_return_val_if_fail (module_path != NULL, FALSE);

  if (pyloader->priv->init_failed)
    return FALSE;

  pathlist = PySys_GetObject ((char *) "path");

#if PY_VERSION_HEX < 0x03000000
  pathstring = PyString_FromString (module_path);
#else
  pathstring = PyUnicode_FromString (module_path);
#endif

  if (PySequence_Contains (pathlist, pathstring) == 0)
    PyList_Insert (pathlist, 0, pathstring);
  Py_DECREF (pathstring);

  return TRUE;
}

#if PY_VERSION_HEX >= 0x03000000
static wchar_t *
peas_wchar_from_str (const gchar *str)
{
  wchar_t *outbuf;
  gsize argsize, count;

  argsize = mbstowcs (NULL, str, 0);
  if (argsize == (gsize)-1)
    {
      g_warning ("Could not convert argument to wchar_t string.");
      return NULL;
    }

  outbuf = g_new (wchar_t, argsize + 1);
  count = mbstowcs (outbuf, str, argsize + 1);
  if (count == (gsize)-1)
    {
      g_warning ("Could not convert argument to wchar_t string.");
      return NULL;
    }

  return outbuf;
}
#endif

static gboolean
peas_python_init (PeasPluginLoaderPython *loader)
{
  PyObject *mdict, *gobject, *gi, *gettext, *install, *gettext_args;
  gchar *prgname;
#if PY_VERSION_HEX < 0x03000000
  const char *argv[] = { "", NULL };
#else
  wchar_t *argv[] = { L"", NULL };
#endif

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
    {
#if PY_VERSION_HEX < 0x03000000
      argv[0] = prgname;
#else
      argv[0] = peas_wchar_from_str (prgname);
#endif
    }

  /* See http://docs.python.org/c-api/init.html#PySys_SetArgvEx */
#if PY_VERSION_HEX < 0x02060600
  PySys_SetArgv (1, (char**) argv);
  PyRun_SimpleString ("import sys; sys.path.pop(0)\n");
#elif PY_VERSION_HEX < 0x03000000
  PySys_SetArgvEx (1, (char**) argv, 0);
#elif PY_VERSION_HEX < 0x03010300
  PySys_SetArgv (1, argv);
  PyRun_SimpleString ("import sys; sys.path.pop(0)\n");
  g_free (argv[0]);
#else
  PySys_SetArgvEx (1, argv, 0);
  g_free (argv[0]);
#endif

  /* Note that we don't call this with the GIL held, since we haven't initialised pygobject yet */
  peas_plugin_loader_python_add_module_path (loader, PEAS_PYEXECDIR);

  /* import gobject */
  pygobject_init (PYGOBJECT_MAJOR_VERSION, PYGOBJECT_MINOR_VERSION, PYGOBJECT_MICRO_VERSION);
  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import pygobject");

      goto python_init_error;
    }

  /* Initialize support for threads */
  pyg_enable_threads ();

  gobject = PyImport_ImportModule ("gobject");
  if (gobject == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import gobject");

      goto python_init_error;
    }

  mdict = PyModule_GetDict (gobject);
  PyGObject_Type = PyDict_GetItemString (mdict, "GObject");
  if (PyGObject_Type == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "get gobject.GObject");

      goto python_init_error;
    }

  gi = PyImport_ImportModule ("gi");
  if (gi == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import gi");

      goto python_init_error;
    }

  /* i18n support */
  gettext = PyImport_ImportModule ("gettext");
  if (gettext == NULL)
    {
      g_warning ("Error initializing Python interpreter: could not "
                 "import gettext");

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
             "required by libpeas and try again");

  PyErr_Clear ();

  peas_python_shutdown (loader);

  return FALSE;
}

static void
destroy_python_info (PythonInfo *info)
{
  PyGILState_STATE state = pyg_gil_state_ensure ();

  Py_DECREF (info->module);

  pyg_gil_state_release (state);

  g_free (info);
}

static void
peas_plugin_loader_python_init (PeasPluginLoaderPython *pyloader)
{
  pyloader->priv = G_TYPE_INSTANCE_GET_PRIVATE (pyloader,
                                                PEAS_TYPE_PLUGIN_LOADER_PYTHON,
                                                PeasPluginLoaderPythonPrivate);

  /* initialize python interpreter */
  peas_python_init (pyloader);

  /* loaded_plugins maps PeasPluginInfo to a PythonInfo */
  pyloader->priv->loaded_plugins = g_hash_table_new_full (g_direct_hash,
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

  loader_class->load = peas_plugin_loader_python_load;
  loader_class->unload = peas_plugin_loader_python_unload;
  loader_class->create_extension = peas_plugin_loader_python_create_extension;
  loader_class->provides_extension = peas_plugin_loader_python_provides_extension;
  loader_class->garbage_collect = peas_plugin_loader_python_garbage_collect;

  g_type_class_add_private (object_class, sizeof (PeasPluginLoaderPythonPrivate));
}
