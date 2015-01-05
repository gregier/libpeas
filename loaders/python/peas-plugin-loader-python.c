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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-plugin-loader-python.h"
#include "peas-python-internal.h"
#include "libpeas/peas-plugin-info-priv.h"

/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * glib-object.h, so we unset it here to avoid a warning. Yep, that's bad.
 */
#undef _POSIX_C_SOURCE
#include <pygobject.h>
#include <Python.h>
#include <signal.h>

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

typedef struct {
  GHashTable *loaded_plugins;
  guint n_loaded_plugins;
  guint idle_gc;
  guint init_failed : 1;
  guint must_finalize_python : 1;
  PyThreadState *py_thread_state;
  PeasPythonInternal *internal;
} PeasPluginLoaderPythonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (PeasPluginLoaderPython,
                            peas_plugin_loader_python,
                            PEAS_TYPE_PLUGIN_LOADER)

#define GET_PRIV(o) \
  (peas_plugin_loader_python_get_instance_private (o))

static
G_DEFINE_QUARK (peas-extension-type, extension_type)

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_PYTHON);
}

/* NOTE: This must be called with the GIL held */
static PyTypeObject *
find_python_extension_type (GType     exten_type,
                            PyObject *pymodule)
{
  PyObject *pygtype, *pytype;
  PyObject *locals, *key, *value;
  Py_ssize_t pos = 0;

  locals = PyModule_GetDict (pymodule);

  pygtype = pyg_type_wrapper_new (exten_type);
  pytype = PyObject_GetAttrString (pygtype, "pytype");
  g_warn_if_fail (pytype != NULL);

  if (pytype != NULL && pytype != Py_None)
    {
      while (PyDict_Next (locals, &pos, &key, &value))
        {
          if (!PyType_Check (value))
            continue;

          switch (PyObject_IsSubclass (value, pytype))
            {
            case 1:
              Py_DECREF (pytype);
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

  Py_DECREF (pytype);
  Py_DECREF (pygtype);

  return NULL;
}

/* C equivalent of
 *    import sys
 *    sys.path.insert(0, module_path)
 */
/* NOTE: This must be called with the GIL held */
static gboolean
add_module_path (PeasPluginLoaderPython *pyloader,
                 const gchar            *module_path)
{
  PyObject *pathlist, *pathstring;
  gboolean success = TRUE;

  g_return_val_if_fail (PEAS_IS_PLUGIN_LOADER_PYTHON (pyloader), FALSE);
  g_return_val_if_fail (module_path != NULL, FALSE);

  pathlist = PySys_GetObject ((char *) "path");
  if (pathlist == NULL)
    return FALSE;

#if PY_VERSION_HEX < 0x03000000
  pathstring = PyString_FromString (module_path);
#else
  pathstring = PyUnicode_FromString (module_path);
#endif

  if (pathstring == NULL)
    return FALSE;

  switch (PySequence_Contains (pathlist, pathstring))
    {
    case 0:
      success = PyList_Insert (pathlist, 0, pathstring) >= 0;
      break;
    case 1:
      break;
    case -1:
    default:
      success = FALSE;
      break;
    }

  Py_DECREF (pathstring);
  return success;
}

/* NOTE: This must be called with the GIL held */
static void
destroy_python_info (gpointer data)
{
  PyObject *pymodule = data;

  Py_XDECREF (pymodule);
}

static gboolean
peas_plugin_loader_python_provides_extension (PeasPluginLoader *loader,
                                              PeasPluginInfo   *info,
                                              GType             exten_type)
{
  PyObject *pymodule = info->loader_data;
  PyTypeObject *extension_type;
  PyGILState_STATE state = PyGILState_Ensure ();

  extension_type = find_python_extension_type (exten_type, pymodule);

  PyGILState_Release (state);
  return extension_type != NULL;
}

static PeasExtension *
peas_plugin_loader_python_create_extension (PeasPluginLoader *loader,
                                            PeasPluginInfo   *info,
                                            GType             exten_type,
                                            guint             n_parameters,
                                            GParameter       *parameters)
{
  PyObject *pymodule = info->loader_data;
  PyTypeObject *pytype;
  GType the_type;
  GObject *object = NULL;
  PyObject *pyobject;
  PyObject *pyplinfo;
  PyGILState_STATE state = PyGILState_Ensure ();

  pytype = find_python_extension_type (exten_type, pymodule);

  if (pytype == NULL)
    goto out;

  the_type = pyg_type_from_object ((PyObject *) pytype);

  if (the_type == G_TYPE_INVALID)
    goto out;

  if (!g_type_is_a (the_type, exten_type))
    {
      g_warn_if_fail (g_type_is_a (the_type, exten_type));
      goto out;
    }

  object = g_object_newv (the_type, n_parameters, parameters);

  if (!object)
    goto out;

  /* We have to remember which interface we are instantiating
   * for the deprecated peas_extension_get_extension_type().
   */
  g_object_set_qdata (object, extension_type_quark (),
                      GSIZE_TO_POINTER (exten_type));

  pyobject = pygobject_new (object);
  pyplinfo = pyg_boxed_new (PEAS_TYPE_PLUGIN_INFO, info, TRUE, TRUE);

  /* Set the plugin info as an attribute of the instance */
  if (PyObject_SetAttrString (pyobject, "plugin_info", pyplinfo) == -1)
    {
      g_warning ("Failed to set 'plugin_info' for '%s'",
                 g_type_name (the_type));

      if (PyErr_Occurred ())
        PyErr_Print ();

      g_clear_object (&object);
    }

  Py_DECREF (pyplinfo);
  Py_DECREF (pyobject);

out:

  PyGILState_Release (state);
  return object;
}

static gboolean
peas_plugin_loader_python_load (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state = PyGILState_Ensure ();
  PyObject *pymodule = NULL;

  if (!g_hash_table_lookup_extended (priv->loaded_plugins,
                                     info->filename,
                                     NULL, (gpointer *) &pymodule))
    {
      const gchar *module_dir, *module_name;

      module_dir = peas_plugin_info_get_module_dir (info);
      module_name = peas_plugin_info_get_module_name (info);

      /* We don't support multiple Python interpreter states */
      if (PyDict_GetItemString (PyImport_GetModuleDict (), module_name))
        {
          g_warning ("Error loading plugin '%s': "
                     "module name '%s' has already been used",
                     info->filename, module_name);
        }
      else if (!add_module_path (pyloader, module_dir))
        {
          g_warning ("Error loading plugin '%s': "
                     "failed to add module path '%s'",
                     module_name, module_dir);
        }
      else
        {
          PyObject *fromlist;

          /* We need a fromlist to be able to
           * import modules with a '.' in the name
           */
          fromlist = PyTuple_New (0);

          pymodule = PyImport_ImportModuleEx ((gchar *) module_name,
                                              NULL, NULL, fromlist);
          Py_DECREF (fromlist);
        }

      if (PyErr_Occurred ())
        PyErr_Print ();

      g_hash_table_insert (priv->loaded_plugins,
                           g_strdup (info->filename), pymodule);
    }

  if (pymodule != NULL)
    {
      info->loader_data = pymodule;
      priv->n_loaded_plugins += 1;
    }

  PyGILState_Release (state);
  return pymodule != NULL;
}

static void
peas_plugin_loader_python_unload (PeasPluginLoader *loader,
                                  PeasPluginInfo   *info)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state = PyGILState_Ensure ();

  /* Only unref the Python module when the
   * loader is finalized as Python keeps a ref anyways
   */

  /* We have to use this as a hook as the
   * loader will not be finalized by applications
   */
  if (--priv->n_loaded_plugins == 0)
    peas_python_internal_call (priv->internal, "all_plugins_unloaded");

  info->loader_data = NULL;
  PyGILState_Release (state);
}

static gboolean
run_gc (PeasPluginLoaderPython *pyloader)
{
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state = PyGILState_Ensure ();

  while (PyGC_Collect ())
    ;

  priv->idle_gc = 0;

  PyGILState_Release (state);
  return FALSE;
}

static void
peas_plugin_loader_python_garbage_collect (PeasPluginLoader *loader)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state = PyGILState_Ensure ();

  /* We both run the GC right now and we schedule
   * a further collection in the main loop.
   */
  while (PyGC_Collect ())
    ;

  if (priv->idle_gc == 0)
    {
      priv->idle_gc = g_idle_add ((GSourceFunc) run_gc, pyloader);
      g_source_set_name_by_id (priv->idle_gc, "[libpeas] run_gc");
    }

  PyGILState_Release (state);
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

#ifdef HAVE_SIGACTION
static void
default_sigint (int sig)
{
  struct sigaction sigint;

  /* Invoke default sigint handler */
  sigint.sa_handler = SIG_DFL;
  sigint.sa_flags = 0;
  sigemptyset (&sigint.sa_mask);

  sigaction (SIGINT, &sigint, NULL);

  raise (SIGINT);
}
#endif

static gboolean
peas_plugin_loader_python_initialize (PeasPluginLoader *loader)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (loader);
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state = 0;
  long hexversion;
  PyObject *gettext, *result;
  const gchar *prgname;
#if PY_VERSION_HEX < 0x03000000
  const char *argv[] = { NULL, NULL };
#else
  wchar_t *argv[] = { NULL, NULL };
#endif

  /* We can't support multiple Python interpreter states:
   * https://bugzilla.gnome.org/show_bug.cgi?id=677091
   */

  /* We are trying to initialize Python for the first time,
     set init_failed to FALSE only if the entire initialization process
     ends with success */
  priv->init_failed = TRUE;

  /* Python initialization */
  if (Py_IsInitialized ())
    {
      state = PyGILState_Ensure ();
    }
  else
    {
#ifdef HAVE_SIGACTION
      struct sigaction sigint;

      /* We are going to install a signal handler for SIGINT if the current
         signal handler for sigint is SIG_DFL. We do this because even if
         Py_InitializeEx will not set the signal handlers, the 'signal' module
         (which can be used by plugins for various reasons) will install a
         SIGINT handler when imported, if SIGINT is set to SIG_DFL. Our
         override will simply call the default SIGINT handler in the end. */
      sigaction (SIGINT, NULL, &sigint);

      if (sigint.sa_handler == SIG_DFL)
        {
          sigemptyset (&sigint.sa_mask);
          sigint.sa_flags = 0;
          sigint.sa_handler = default_sigint;

          sigaction (SIGINT, &sigint, NULL);
        }
#endif

      Py_InitializeEx (FALSE);
      priv->must_finalize_python = TRUE;
    }

  hexversion = PyLong_AsLong (PySys_GetObject ((char *) "hexversion"));

#if PY_VERSION_HEX < 0x03000000
  if (hexversion >= 0x03000000)
#else
  if (hexversion < 0x03000000)
#endif
    {
      g_critical ("Attempting to mix incompatible Python versions");

      goto python_init_error;
    }

  prgname = g_get_prgname ();
  prgname = prgname == NULL ? "" : prgname;

#if PY_VERSION_HEX < 0x03000000
  argv[0] = prgname;
#else
  argv[0] = peas_wchar_from_str (prgname);
#endif

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

  if (!add_module_path (pyloader, PEAS_PYEXECDIR))
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to add the module path");

      goto python_init_error;
    }

  /* Initialize PyGObject */
  pygobject_init (PYGOBJECT_MAJOR_VERSION,
                  PYGOBJECT_MINOR_VERSION,
                  PYGOBJECT_MICRO_VERSION);

  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "PyGObject initialization failed");

      goto python_init_error;
    }

  /* Initialize support for threads */
  pyg_enable_threads ();
  PyEval_InitThreads ();

  /* Only redirect warnings when python was not already initialized */
  if (!priv->must_finalize_python)
    pyg_disable_warning_redirections ();

  /* i18n support */
  gettext = PyImport_ImportModule ("gettext");
  if (gettext == NULL)
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to import gettext");

      goto python_init_error;
    }

  result = PyObject_CallMethod (gettext, "install", "ss",
                                GETTEXT_PACKAGE, PEAS_LOCALEDIR);
  Py_XDECREF (result);

  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to install gettext");

      goto python_init_error;
    }

  priv->internal = peas_python_internal_new ();
  if (priv->internal == NULL)
    {
      /* Already warned */
      goto python_init_error;
    }

  /* Python has been successfully initialized */
  priv->init_failed = FALSE;

  if (!priv->must_finalize_python)
    PyGILState_Release (state);
  else
    priv->py_thread_state = PyEval_SaveThread ();

  /* loaded_plugins maps PeasPluginInfo:filename to a PyObject */
  priv->loaded_plugins = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                g_free, destroy_python_info);

  return TRUE;

python_init_error:

  if (PyErr_Occurred ())
    PyErr_Print ();

  g_warning ("Please check the installation of all the Python "
             "related packages required by libpeas and try again");

  if (!priv->must_finalize_python)
    PyGILState_Release (state);

  return FALSE;
}

static void
peas_plugin_loader_python_init (PeasPluginLoaderPython *pyloader)
{
}

static void
peas_plugin_loader_python_finalize (GObject *object)
{
  PeasPluginLoaderPython *pyloader = PEAS_PLUGIN_LOADER_PYTHON (object);
  PeasPluginLoaderPythonPrivate *priv = GET_PRIV (pyloader);
  PyGILState_STATE state;

  if (!Py_IsInitialized ())
    goto out;

  g_warn_if_fail (priv->n_loaded_plugins == 0);

  if (priv->loaded_plugins != NULL)
    {
      state = PyGILState_Ensure ();
      g_hash_table_destroy (priv->loaded_plugins);
      PyGILState_Release (state);
    }

  if (priv->internal != NULL && !priv->init_failed)
    {
      state = PyGILState_Ensure ();
      peas_python_internal_free (priv->internal);
      PyGILState_Release (state);
    }

  if (priv->py_thread_state)
    PyEval_RestoreThread (priv->py_thread_state);

  if (priv->idle_gc != 0)
    g_source_remove (priv->idle_gc);

  if (!priv->init_failed)
    run_gc (pyloader);

  if (priv->must_finalize_python)
    {
      if (!priv->init_failed)
        PyGILState_Ensure ();

      Py_Finalize ();
    }

out:

  G_OBJECT_CLASS (peas_plugin_loader_python_parent_class)->finalize (object);
}

static void
peas_plugin_loader_python_class_init (PeasPluginLoaderPythonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  object_class->finalize = peas_plugin_loader_python_finalize;

  loader_class->initialize = peas_plugin_loader_python_initialize;
  loader_class->load = peas_plugin_loader_python_load;
  loader_class->unload = peas_plugin_loader_python_unload;
  loader_class->create_extension = peas_plugin_loader_python_create_extension;
  loader_class->provides_extension = peas_plugin_loader_python_provides_extension;
  loader_class->garbage_collect = peas_plugin_loader_python_garbage_collect;
}
