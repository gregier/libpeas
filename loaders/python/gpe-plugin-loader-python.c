/*
 * gpe-plugin-loader-python.c
 * This file is part of libgpe
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
 * Copyright (C) 2009 - Steve Frécinaux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gpe-plugin-loader-python.h"

#if 0
#define NO_IMPORT_PYGOBJECT
#define NO_IMPORT_PYGTK
#endif

#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>
#include <signal.h>
#include "config.h"

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

#define GPE_PLUGIN_LOADER_PYTHON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GPE_TYPE_PLUGIN_LOADER_PYTHON, GPEPluginLoaderPythonPrivate))

struct _GPEPluginLoaderPythonPrivate
{
	GHashTable *loaded_plugins;
	guint idle_gc;
	gboolean init_failed;
};

typedef struct
{
	PyObject *type;
	PyObject *instance;
	gchar    *path;
} PythonInfo;

static void gpe_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

/* We retreive this to check for correct class hierarchy */
static PyTypeObject *PyGPEPlugin_Type;

GPE_PLUGIN_LOADER_REGISTER_TYPE (GPEPluginLoaderPython, gpe_plugin_loader_python, G_TYPE_OBJECT, gpe_plugin_loader_iface_init);


static PyObject *
find_python_plugin_type (GPEPluginInfo *info,
			 PyObject      *pymodule)
{
	PyObject *locals, *key, *value;
	Py_ssize_t pos = 0;

	locals = PyModule_GetDict (pymodule);

	while (PyDict_Next (locals, &pos, &key, &value))
	{
		if (!PyType_Check(value))
			continue;

		if (PyObject_IsSubclass (value, (PyObject*) PyGPEPlugin_Type))
			return value;
	}

	g_warning ("No GPEPlugin derivative found in Python plugin '%s'",
		   gpe_plugin_info_get_name (info));
	return NULL;
}

static GPEPlugin *
new_plugin_from_info (GPEPluginLoaderPython *loader,
		      GPEPluginInfo         *info)
{
	PythonInfo *pyinfo;
	PyTypeObject *pytype;
	PyObject *pyobject;
	PyGObject *pygobject;
	GPEPlugin *instance;
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
		         gpe_plugin_info_get_name (info));
		return NULL;
	}

	pygobject = (PyGObject *) pyobject;

	if (pygobject->obj != NULL)
	{
		Py_DECREF (pyobject);
		g_error ("Could not create instance for %s (GObject already initialized).",
			 gpe_plugin_info_get_name (info));
		return NULL;
	}

	pygobject_construct (pygobject, NULL);

	if (pygobject->obj == NULL)
	{
		g_error ("Could not create instance for %s (GObject not constructed).",
			 gpe_plugin_info_get_name (info));
		Py_DECREF (pyobject);

		return NULL;
	}

	/* now call tp_init manually */
	if (PyType_IsSubtype (pyobject->ob_type, pytype) && pyobject->ob_type->tp_init != NULL)
	{
		emptyarg = PyTuple_New (0);
		pyobject->ob_type->tp_init (pyobject, emptyarg, NULL);
		Py_DECREF (emptyarg);
	}

	instance = GPE_PLUGIN (pygobject->obj);
	pyinfo->instance = (PyObject *) pygobject;

	/* we return a reference here because the other is owned by python */
	return GPE_PLUGIN (g_object_ref (instance));
}

static GPEPlugin *
add_python_info (GPEPluginLoaderPython *loader,
		 GPEPluginInfo         *info,
		 PyObject              *module,
		 const gchar           *path,
		 PyObject              *type)
{
	PythonInfo *pyinfo;

	pyinfo = g_new (PythonInfo, 1);
	pyinfo->path = g_strdup (path);
	pyinfo->type = type;

	Py_INCREF (pyinfo->type);

	g_hash_table_insert (loader->priv->loaded_plugins, info, pyinfo);

	return new_plugin_from_info (loader, info);
}

static const gchar *
gpe_plugin_loader_iface_get_id (void)
{
	return "Python";
}

static GPEPlugin *
gpe_plugin_loader_iface_load (GPEPluginLoader *loader,
			      GPEPluginInfo   *info,
			      const gchar     *path,
			      const gchar     *datadir)
{
	GPEPluginLoaderPython *pyloader = GPE_PLUGIN_LOADER_PYTHON (loader);
	PyObject *main_module, *main_locals, *pytype;
	PyObject *pymodule, *fromlist;
	gchar *module_name;
	GPEPlugin *result;

	if (pyloader->priv->init_failed)
	{
		g_warning ("Cannot load python plugin Python '%s' since libgpe was"
			   "not able to initialize the Python interpreter.",
			   gpe_plugin_info_get_name (info));
		return NULL;
	}

	/* see if py definition for the plugin is already loaded */
	result = new_plugin_from_info (pyloader, info);

	if (result != NULL)
		return result;

	main_module = PyImport_AddModule ("libgpe.plugins");
	if (main_module == NULL)
	{
		g_warning ("Could not get libgpe.plugins.");
		return NULL;
	}

	/* If we have a special path, we register it */
	if (path != NULL)
	{
		PyObject *sys_path = PySys_GetObject ("path");
		PyObject *pypath = PyString_FromString (path);

		if (PySequence_Contains (sys_path, pypath) == 0)
			PyList_Insert (sys_path, 0, pypath);

		Py_DECREF (pypath);
	}

	main_locals = PyModule_GetDict (main_module);

	/* we need a fromlist to be able to import modules with a '.' in the
	   name. */
	fromlist = PyTuple_New (0);
	module_name = g_strdup (gpe_plugin_info_get_module_name (info));

	pymodule = PyImport_ImportModuleEx (module_name,
					    main_locals,
					    main_locals,
					    fromlist);

	Py_DECREF(fromlist);

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
		return add_python_info (pyloader, info, pymodule, path, pytype);

	return NULL;
}

static void
gpe_plugin_loader_iface_unload (GPEPluginLoader *loader,
				GPEPluginInfo   *info)
{
	GPEPluginLoaderPython *pyloader = GPE_PLUGIN_LOADER_PYTHON (loader);
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
run_gc (GPEPluginLoaderPython *loader)
{
	while (PyGC_Collect ())
		;

	loader->priv->idle_gc = 0;
	return FALSE;
}

static void
gpe_plugin_loader_iface_garbage_collect (GPEPluginLoader *loader)
{
	GPEPluginLoaderPython *pyloader;

	if (!Py_IsInitialized ())
		return;

	pyloader = GPE_PLUGIN_LOADER_PYTHON (loader);

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
gpe_plugin_loader_iface_init (gpointer g_iface,
			      gpointer iface_data)
{
	GPEPluginLoaderInterface *iface = (GPEPluginLoaderInterface *) g_iface;

	iface->get_id = gpe_plugin_loader_iface_get_id;
	iface->load = gpe_plugin_loader_iface_load;
	iface->unload = gpe_plugin_loader_iface_unload;
	iface->garbage_collect = gpe_plugin_loader_iface_garbage_collect;
}

static void
gpe_python_shutdown (GPEPluginLoaderPython *loader)
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
 *    import pygtk
 *    pygtk.require ("2.0")
 */
static gboolean
gpe_check_pygtk2 (void)
{
	PyObject *pygtk, *mdict, *require;

	/* pygtk.require("2.0") */
	pygtk = PyImport_ImportModule ("pygtk");
	if (pygtk == NULL)
	{
		g_warning ("Error initializing Python interpreter: could not import pygtk.");
		return FALSE;
	}

	mdict = PyModule_GetDict (pygtk);
	require = PyDict_GetItemString (mdict, "require");
	PyObject_CallObject (require, Py_BuildValue ("(S)", PyString_FromString ("2.0")));
	if (PyErr_Occurred ())
	{
		g_warning ("Error initializing Python interpreter: pygtk 2 is required.");
		return FALSE;
	}

	return TRUE;
}

/* Note: the following two functions are needed because
 * init_pyobject and init_pygtk which are *macros* which in case
 * case of error set the PyErr and then make the calling
 * function return behind our back.
 * It's up to the caller to check the result with PyErr_Occurred ()
 */
static void
gpe_init_pygobject (void)
{
	init_pygobject_check (2, 11, 5); /* FIXME: get from config */
}

static void
gpe_init_pygtk (void)
{
	PyObject *gtk, *mdict, *version, *required_version;

	init_pygtk ();

	/* there isn't init_pygtk_check(), do the version
	 * check ourselves */
	gtk = PyImport_ImportModule ("gtk");
	mdict = PyModule_GetDict (gtk);
	version = PyDict_GetItemString (mdict, "pygtk_version");
	if (!version)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGObject version too old");
		return;
	}

	required_version = Py_BuildValue ("(iii)", 2, 4, 0); /* FIXME */

	if (PyObject_Compare (version, required_version) == -1)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGObject version too old");
		Py_DECREF (required_version);
		return;
	}

	Py_DECREF (required_version);
}

static void
gpe_init_libgpe (void)
{
	PyObject *libgpe, *mdict, *version, *required_version, *pluginsmodule;

	libgpe = PyImport_ImportModule("libgpe");
	if (libgpe == NULL)
	{
		PyErr_SetString (PyExc_ImportError,
				 "could not import libgpe module");
		return;
	}

	mdict = PyModule_GetDict (libgpe);
	version = PyDict_GetItemString (mdict, "version");
	if (!version)
	{
		PyErr_SetString (PyExc_ImportError,
				 "could not get libgpe module version");
		return;
	}

	required_version = Py_BuildValue ("(iii)",
					  GPE_MAJOR_VERSION,
					  GPE_MINOR_VERSION,
					  GPE_MICRO_VERSION);

	if (PyObject_Compare (version, required_version) == -1)
	{
		PyErr_SetString (PyExc_ImportError,
				 "libgpe module version too old");
		Py_DECREF (required_version);
		return;
	}

	Py_DECREF (required_version);

	/* Retrieve the Python type for libgpe.Plugin */
	PyGPEPlugin_Type = (PyTypeObject *) PyDict_GetItemString (mdict, "Plugin");
	if (PyGPEPlugin_Type == NULL)
	{
		PyErr_SetString (PyExc_ImportError,
				 "could not find libgpe.Plugin");
		return;
	}

	/* initialize empty libgpe.plugins module */
	pluginsmodule = Py_InitModule ("libgpe.plugins", NULL);
	PyDict_SetItemString (mdict, "plugins", pluginsmodule);
}

static gboolean
gpe_python_init (GPEPluginLoaderPython *loader)
{
	PyObject *mdict, *gettext, *install, *gettext_args;
	struct sigaction old_sigint;
	gint res;
	char *argv[] = { "libgpe", NULL };

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

	/* Save old handler */
	res = sigaction (SIGINT, NULL, &old_sigint);
	if (res != 0)
	{
		g_warning ("Error initializing Python interpreter: cannot get "
		           "handler to SIGINT signal (%s)",
		           g_strerror (errno));

		return FALSE;
	}

	/* Python initialization */
	Py_Initialize ();

	/* Restore old handler */
	res = sigaction (SIGINT, &old_sigint, NULL);
	if (res != 0)
	{
		g_warning ("Error initializing Python interpreter: cannot restore "
		           "handler to SIGINT signal (%s).",
		           g_strerror (errno));

		goto python_init_error;
	}

	PySys_SetArgv (1, argv);

	if (!gpe_check_pygtk2 ())
	{
		/* Warning message already printed in check_pygtk2 */
		goto python_init_error;
	}

	/* import gobject */
	gpe_init_pygobject ();
	if (PyErr_Occurred ())
	{
		g_warning ("Error initializing Python interpreter: could not import pygobject.");

		goto python_init_error;
	}

	/* import gtk */
	gpe_init_pygtk ();
	if (PyErr_Occurred ())
	{
		g_warning ("Error initializing Python interpreter: could not import pygtk.");

		goto python_init_error;
	}

	/* import libgpe */
	gpe_init_libgpe ();
	if (PyErr_Occurred ())
	{
		PyErr_Print ();

		g_warning ("Error initializing Python interpreter: could not import libgpe module.");

		goto python_init_error;
	}

	/* i18n support */
	gettext = PyImport_ImportModule ("gettext");
	if (gettext == NULL)
	{
		g_warning ("Error initializing Python interpreter: could not import gettext.");

		goto python_init_error;
	}

	mdict = PyModule_GetDict (gettext);
	install = PyDict_GetItemString (mdict, "install");
	gettext_args = Py_BuildValue ("ss", GETTEXT_PACKAGE, GPE_LOCALEDIR);
	PyObject_CallObject (install, gettext_args);
	Py_DECREF (gettext_args);

	/* Python has been successfully initialized */
	loader->priv->init_failed = FALSE;

	return TRUE;

python_init_error:

	g_warning ("Please check the installation of all the Python related packages required "
	           "by libgpe and try again.");

	PyErr_Clear ();

	gpe_python_shutdown (loader);

	return FALSE;
}

static void
destroy_python_info (PythonInfo *info)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	Py_XDECREF (info->type);
	pyg_gil_state_release (state);

	g_free (info->path);
	g_free (info);
}

static void
gpe_plugin_loader_python_init (GPEPluginLoaderPython *self)
{
	self->priv = GPE_PLUGIN_LOADER_PYTHON_GET_PRIVATE (self);

	/* initialize python interpreter */
	gpe_python_init (self);

	/* loaded_plugins maps EggPluginsInfo to a PythonInfo */
	self->priv->loaded_plugins = g_hash_table_new_full (g_direct_hash,
						            g_direct_equal,
						            NULL,
						            (GDestroyNotify) destroy_python_info);
}

static void
gpe_plugin_loader_python_finalize (GObject *object)
{
	GPEPluginLoaderPython *pyloader = GPE_PLUGIN_LOADER_PYTHON (object);

	g_hash_table_destroy (pyloader->priv->loaded_plugins);
	gpe_python_shutdown (pyloader);

	G_OBJECT_CLASS (gpe_plugin_loader_python_parent_class)->finalize (object);
}

static void
gpe_plugin_loader_python_class_init (GPEPluginLoaderPythonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gpe_plugin_loader_python_finalize;

	g_type_class_add_private (object_class, sizeof (GPEPluginLoaderPythonPrivate));
}

static void
gpe_plugin_loader_python_class_finalize (GPEPluginLoaderPythonClass *klass)
{
}

GPEPluginLoaderPython *
gpe_plugin_loader_python_new ()
{
	GObject *loader = g_object_new (GPE_TYPE_PLUGIN_LOADER_PYTHON, NULL);

	return GPE_PLUGIN_LOADER_PYTHON (loader);
}

