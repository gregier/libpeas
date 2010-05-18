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

#include "peas-plugin-loader-python.h"

#if 0
#define NO_IMPORT_PYGOBJECT
#define NO_IMPORT_PYGTK
#endif

/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * glib-object.h, so we unset it here to avoid a warning. Yep, that's bad. */
#undef _POSIX_C_SOURCE
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
  PyObject *type;
  PyObject *instance;
} PythonInfo;

static gboolean   peas_plugin_loader_python_add_module_path (PeasPluginLoaderPython *self,
                                                             const gchar            *module_path);

/* We retreive this to check for correct class hierarchy */
static PyTypeObject *PyPeasPlugin_Type;

PEAS_PLUGIN_LOADER_REGISTER_TYPE (PeasPluginLoaderPython, peas_plugin_loader_python);

static PyObject *
find_python_plugin_type (PeasPluginInfo *info,
                         PyObject       *pymodule)
{
  PyObject *locals, *key, *value;
  Py_ssize_t pos = 0;

  locals = PyModule_GetDict (pymodule);

  while (PyDict_Next (locals, &pos, &key, &value))
    {
      if (!PyType_Check (value))
        continue;

      if (PyObject_IsSubclass (value, (PyObject *) PyPeasPlugin_Type))
        return value;
    }

  g_warning ("No PeasPlugin derivative found in Python plugin '%s'",
             peas_plugin_info_get_name (info));
  return NULL;
}

static PeasPlugin *
new_plugin_from_info (PeasPluginLoaderPython *loader,
                      PeasPluginInfo         *info)
{
  PythonInfo *pyinfo;
  PyTypeObject *pytype;
  PyObject *pyobject;
  PyGObject *pygobject;
  PeasPlugin *instance;
  PyObject *emptyarg;

  pyinfo = (PythonInfo *) g_hash_table_lookup (loader->priv->loaded_plugins, info);

  if (pyinfo == NULL)
    return NULL;

  pytype = (PyTypeObject *) pyinfo->type;

  if (pytype->tp_new == NULL)
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

  pygobject_construct (pygobject, "plugin-info", info, NULL);

  if (pygobject->obj == NULL)
    {
      g_error ("Could not create instance for %s (GObject not constructed).",
               peas_plugin_info_get_name (info));
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

  instance = PEAS_PLUGIN (pygobject->obj);
  pyinfo->instance = (PyObject *) pygobject;

  /* we return a reference here because the other is owned by python */
  return PEAS_PLUGIN (g_object_ref (instance));
}

static PeasPlugin *
add_python_info (PeasPluginLoaderPython *loader,
                 PeasPluginInfo         *info,
                 PyObject               *module,
                 PyObject               *type)
{
  PythonInfo *pyinfo;

  pyinfo = g_new (PythonInfo, 1);
  pyinfo->type = type;

  Py_INCREF (pyinfo->type);

  g_hash_table_insert (loader->priv->loaded_plugins, info, pyinfo);

  return new_plugin_from_info (loader, info);
}

static void
peas_plugin_loader_python_add_module_directory (PeasPluginLoader *loader,
                                                const gchar      *module_dir)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);

  g_debug ("Adding %s as a module path for the python loader.", module_dir);
  peas_plugin_loader_python_add_module_path (pyloader, module_dir);
}

static PeasPlugin *
peas_plugin_loader_python_load (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PyObject *main_module, *main_locals, *pytype;
  PyObject *pymodule, *fromlist;
  gchar *module_name;
  PeasPlugin *result;

  if (pyloader->priv->init_failed)
    {
      g_warning ("Cannot load python plugin Python '%s' since libpeas was"
                 "not able to initialize the Python interpreter.",
                 peas_plugin_info_get_name (info));
      return NULL;
    }

  /* see if py definition for the plugin is already loaded */
  result = new_plugin_from_info (pyloader, info);

  if (result != NULL)
    return result;

  main_module = PyImport_AddModule ("libpeas.plugins");
  if (main_module == NULL)
    {
      g_warning ("Could not get libpeas.plugins.");
      return NULL;
    }

  /* If we have a special path, we register it */
  peas_plugin_loader_python_add_module_path (pyloader,
                                             peas_plugin_info_get_module_dir (info));

  main_locals = PyModule_GetDict (main_module);

  /* we need a fromlist to be able to import modules with a '.' in the
     name. */
  fromlist = PyTuple_New (0);
  module_name = g_strdup (peas_plugin_info_get_module_name (info));

  pymodule = PyImport_ImportModuleEx (module_name,
                                      main_locals,
                                      main_locals,
                                      fromlist);

  Py_DECREF (fromlist);

  if (!pymodule)
    {
      g_free (module_name);
      PyErr_Print ();
      return NULL;
    }

  PyDict_SetItemString (main_locals, module_name, pymodule);
  g_free (module_name);

  pytype = find_python_plugin_type (info, pymodule);

  if (pytype)
    return add_python_info (pyloader, info, pymodule, pytype);

  return NULL;
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
  Py_XDECREF (pyinfo->instance);
  pyg_gil_state_release (state);

  pyinfo->instance = NULL;
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

static void
peas_init_libpeas (void)
{
  PyObject *libpeas, *mdict, *version, *required_version, *pluginsmodule;

  libpeas = PyImport_ImportModule ("libpeas");
  if (libpeas == NULL)
    {
      PyErr_SetString (PyExc_ImportError, "could not import libpeas module");
      return;
    }

  mdict = PyModule_GetDict (libpeas);
  version = PyDict_GetItemString (mdict, "version");
  if (!version)
    {
      PyErr_SetString (PyExc_ImportError,
                       "could not get libpeas module version");
      return;
    }

  required_version = Py_BuildValue ("(iii)",
                                    PEAS_MAJOR_VERSION,
                                    PEAS_MINOR_VERSION, PEAS_MICRO_VERSION);

  if (PyObject_Compare (version, required_version) == -1)
    {
      PyErr_SetString (PyExc_ImportError, "libpeas module version too old");
      Py_DECREF (required_version);
      return;
    }

  Py_DECREF (required_version);

  /* Retrieve the Python type for libpeas.Plugin */
  PyPeasPlugin_Type = (PyTypeObject *) PyDict_GetItemString (mdict, "Plugin");
  if (PyPeasPlugin_Type == NULL)
    {
      PyErr_SetString (PyExc_ImportError, "could not find libpeas.Plugin");
      return;
    }

  /* initialize empty libpeas.plugins module */
  pluginsmodule = Py_InitModule ("libpeas.plugins", NULL);
  PyDict_SetItemString (mdict, "plugins", pluginsmodule);
}

static gboolean
peas_python_init (PeasPluginLoaderPython *loader)
{
  PyObject *mdict, *gettext, *install, *gettext_args;
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
      g_warning
        ("Error initializing Python interpreter: could not import pygobject.");

      goto python_init_error;
    }

  /* import libpeas */
  peas_init_libpeas ();
  if (PyErr_Occurred ())
    {
      PyErr_Print ();

      g_warning
        ("Error initializing Python interpreter: could not import libpeas module.");

      goto python_init_error;
    }

  /* i18n support */
  gettext = PyImport_ImportModule ("gettext");
  if (gettext == NULL)
    {
      g_warning
        ("Error initializing Python interpreter: could not import gettext.");

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

  g_warning
    ("Please check the installation of all the Python related packages required "
     "by libpeas and try again.");

  PyErr_Clear ();

  peas_python_shutdown (loader);

  return FALSE;
}

static void
destroy_python_info (PythonInfo *info)
{
  PyGILState_STATE state = pyg_gil_state_ensure ();
  Py_XDECREF (info->type);
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
  loader_class->garbage_collect = peas_plugin_loader_python_garbage_collect;

  g_type_class_add_private (object_class,
                            sizeof (PeasPluginLoaderPythonPrivate));
}

static void
peas_plugin_loader_python_class_finalize (PeasPluginLoaderPythonClass *klass)
{
}
