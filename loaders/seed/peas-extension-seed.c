/*
 * peas-extension-seed.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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

#include "peas-extension-seed.h"
#include <libpeas/peas-introspection.h>
#include <libpeas/peas-extension-subclasses.h>
#include <girepository.h>

G_DEFINE_TYPE (PeasExtensionSeed, peas_extension_seed, PEAS_TYPE_EXTENSION);

enum {
  PROP_0,
  PROP_EXTEN_TYPE,
  PROP_JS_CONTEXT,
  PROP_JS_OBJECT,
};

typedef struct {
  GITypeInfo *type_info;
  GIArgument *ptr;
} OutArg;

static void
peas_extension_seed_init (PeasExtensionSeed *sexten)
{
}

static void
peas_extension_seed_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);

  switch (prop_id)
    {
    case PROP_JS_CONTEXT:
      sexten->js_context = g_value_get_pointer (value);
      break;
    case PROP_JS_OBJECT:
      sexten->js_object = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
peas_extension_seed_constructed (GObject *object)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);

  /* We do this here as we can't be sure the context is already set when
   * the "JS_PLUGIN" property is set. */
  seed_context_ref (sexten->js_context);
  seed_value_protect (sexten->js_context, sexten->js_object);
}

static void
peas_extension_seed_finalize (GObject *object)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);

  seed_value_unprotect (sexten->js_context, sexten->js_object);
  seed_context_unref (sexten->js_context);
}

static SeedValue
get_argument (SeedContext ctx,
              GIArgument *arg,
              GITypeInfo *arg_type_info,
              SeedException *exc)
{
  /* Notes: According to GCC 4.4,
   *  - int8, uint8, int16, uint16, short and ushort are promoted to int when passed through '...'
   *  - float is promoted to double when passed through '...'
   */
  switch (g_type_info_get_tag (arg_type_info))
    {
    case GI_TYPE_TAG_VOID:
      g_assert_not_reached ();
      break;
    case GI_TYPE_TAG_BOOLEAN:
      return seed_value_from_boolean (ctx, arg->v_boolean, exc);
    case GI_TYPE_TAG_INT8:
      return seed_value_from_int (ctx, arg->v_int8, exc);
    case GI_TYPE_TAG_UINT8:
      return seed_value_from_uint (ctx, arg->v_uint8, exc);
    case GI_TYPE_TAG_INT16:
      return seed_value_from_int (ctx, arg->v_int16, exc);
    case GI_TYPE_TAG_UINT16:
      return seed_value_from_uint (ctx, arg->v_uint16, exc);
    case GI_TYPE_TAG_INT32:
      return seed_value_from_long (ctx, arg->v_int32, exc);
    case GI_TYPE_TAG_UINT32:
      return seed_value_from_ulong (ctx, arg->v_uint32, exc);
    case GI_TYPE_TAG_INT64:
      return seed_value_from_int64 (ctx, arg->v_int64, exc);
    case GI_TYPE_TAG_UINT64:
      return seed_value_from_uint64 (ctx, arg->v_uint64, exc);
    case GI_TYPE_TAG_FLOAT:
      return seed_value_from_float (ctx, arg->v_float, exc);
    case GI_TYPE_TAG_DOUBLE:
      return seed_value_from_double (ctx, arg->v_double, exc);
      break;

    case GI_TYPE_TAG_GTYPE:
      /* apparently, GType is meant to be a gsize, from gobject/gtype.h in glib */
      return seed_value_from_ulong (ctx, arg->v_size, exc);

    case GI_TYPE_TAG_UTF8:
      return seed_value_from_string (ctx, arg->v_string, exc);
    case GI_TYPE_TAG_FILENAME:
      return seed_value_from_filename (ctx, arg->v_string, exc);

    case GI_TYPE_TAG_INTERFACE:
      return seed_value_from_object (ctx, arg->v_pointer, exc);

    /* FIXME */
    case GI_TYPE_TAG_ARRAY:
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_GHASH:
    case GI_TYPE_TAG_ERROR:
      return seed_make_undefined (ctx);

    default:
      g_return_val_if_reached (seed_make_undefined (ctx));
    }
}

static void
set_return_value (OutArg        *arg,
                  SeedContext    ctx,
                  SeedValue      value,
                  SeedException *exc)
{
  g_debug (G_STRFUNC);
  switch (g_type_info_get_tag (arg->type_info))
    {
    case GI_TYPE_TAG_BOOLEAN:
      arg->ptr->v_boolean = seed_value_to_boolean (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT8:
      arg->ptr->v_int8 = seed_value_to_int (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT8:
      arg->ptr->v_uint8 = seed_value_to_uint (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT16:
      arg->ptr->v_int16 = seed_value_to_int (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT16:
      arg->ptr->v_uint16 = seed_value_to_uint (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT32:
      arg->ptr->v_int32 = seed_value_to_long (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT32:
      arg->ptr->v_uint32 = seed_value_to_ulong (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT64:
      arg->ptr->v_int64 = seed_value_to_int64 (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT64:
      arg->ptr->v_uint64 = seed_value_to_uint64 (ctx, value, exc);
      break;
    case GI_TYPE_TAG_FLOAT:
      arg->ptr->v_float = seed_value_to_float (ctx, value, exc);
      break;
    case GI_TYPE_TAG_DOUBLE:
      arg->ptr->v_double = seed_value_to_double (ctx, value, exc);
      break;

    case GI_TYPE_TAG_GTYPE:
      /* apparently, GType is meant to be a gsize, from gobject/gtype.h in glib */
      arg->ptr->v_size = seed_value_to_ulong (ctx, value, exc);
      break;

    case GI_TYPE_TAG_UTF8:
      arg->ptr->v_string = seed_value_to_string (ctx, value, exc);
      break;
    case GI_TYPE_TAG_FILENAME:
      arg->ptr->v_string = seed_value_to_filename (ctx, value, exc);
      break;

    case GI_TYPE_TAG_INTERFACE:
      arg->ptr->v_pointer = seed_value_to_object (ctx, value, exc);
      break;

    /* FIXME */
    case GI_TYPE_TAG_ARRAY:
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_GHASH:
    case GI_TYPE_TAG_ERROR:
      arg->ptr->v_pointer = NULL;

    default:
      g_return_if_reached ();
    }
}

static gboolean
peas_extension_seed_call (PeasExtension *exten,
                          const gchar   *method_name,
                          GIArgument    *args,
                          GIArgument    *retval)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (exten);
  GType exten_type;
  SeedValue js_method;
  GICallableInfo *func_info;
  GITypeInfo *retval_info;
  gint n_args;
  guint n_in_args, n_out_args, i;
  SeedValue *js_in_args;
  OutArg *out_args;
  SeedValue js_ret, val;
  SeedException exc = NULL;
  gchar *exc_string;

  g_return_val_if_fail (sexten->js_context != NULL, FALSE);
  g_return_val_if_fail (sexten->js_object != NULL, FALSE);

  exten_type = peas_extension_get_extension_type (exten);

  /* Fetch the JS method we want to call */
  js_method = seed_object_get_property (sexten->js_context,
                                        sexten->js_object,
                                        method_name);
  if (seed_value_is_undefined (sexten->js_context, js_method))
    {
      g_warning ("Method '%s.%s' is not defined",
                 g_type_name (exten_type), method_name);
      return FALSE;
    }

  /* We want to display an error if the method is defined but is not a function. */
  if (!seed_value_is_function (sexten->js_context, js_method))
    {
      g_warning ("Method '%s.%s' is not a function",
                 g_type_name (exten_type), method_name);
      return FALSE;
    }

  /* Prepare the arguments */
  func_info = peas_gi_get_method_info (exten_type, method_name);
  if (func_info == NULL)
    return FALSE;

  n_args = g_callable_info_get_n_args (func_info);
  g_return_val_if_fail (n_args >= 0, FALSE);
  n_in_args = 0;
  n_out_args = 0;

  js_in_args = g_new0 (SeedValue, n_args);
  out_args = g_new0 (OutArg, n_args + 1);

  /* We put the return value first in the out tuple, as it seems to be
   * the common behaviour for GI-based bindings */
  retval_info = g_callable_info_get_return_type (func_info);
  if (g_type_info_get_tag (retval_info) != GI_TYPE_TAG_VOID)
    {
      out_args[0].ptr = retval;
      out_args[0].type_info = retval_info;
      g_base_info_ref ((GIBaseInfo *) retval_info);
      n_out_args = 1;
    }
  g_base_info_unref ((GIBaseInfo *) retval_info);

  /* Handle the other arguments */
  for (i = 0; i < n_args && exc == NULL; i++)
    {
      GIArgInfo arg_info;
      GITypeInfo *arg_type_info;

      g_callable_info_load_arg (func_info, i, &arg_info);
      arg_type_info = g_arg_info_get_type (&arg_info);

      switch (g_arg_info_get_direction (&arg_info))
        {
        case GI_DIRECTION_IN:
          js_in_args[n_in_args++] = get_argument (sexten->js_context,
                                                  &args[i],
                                                  arg_type_info,
                                                  &exc);
          break;
        case GI_DIRECTION_OUT:
          out_args[n_out_args].ptr = &args[i];
          out_args[n_out_args].type_info = arg_type_info;
          g_base_info_ref ((GIBaseInfo *) arg_type_info);
          n_out_args++;
          break;
        default:
          seed_make_exception (sexten->js_context,
                               &exc,
                               "dir_not_supported",
                               "Argument direction 'inout' not supported yet");
          break;
        }

      g_base_info_unref ((GIBaseInfo *) arg_type_info);
    }
  if (exc != NULL)
    goto cleanup;

  js_ret = seed_object_call (sexten->js_context,
                             js_method,
                             sexten->js_object,
                             n_in_args,
                             js_in_args,
                             &exc);
  if (exc != NULL)
    goto cleanup;

  switch (n_out_args)
    {
    case 0:
      break;
    case 1:
      set_return_value (&out_args[0], sexten->js_context, js_ret, exc);
      break;
    default:
      if (seed_value_is_object (sexten->js_context, js_ret))
        for (i = 0; i < n_out_args && exc == NULL; i++)
          {
            val = seed_object_get_property_at_index (sexten->js_context, js_ret, i, exc);
            if (exc == NULL)
              set_return_value (&out_args[i], sexten->js_context, val, exc);
          }
    }

cleanup:
  for (i = 0; i < n_out_args; i++)
    g_base_info_unref ((GIBaseInfo *) out_args[i].type_info);
  g_free (out_args);
  g_free (js_in_args);
  g_base_info_unref ((GIBaseInfo *) func_info);

  if (exc == NULL)
    return TRUE;

  exc_string = seed_exception_to_string (sexten->js_context, exc);
  g_warning ("Seed Exception: %s", exc_string);
  g_free (exc_string);
  return FALSE;
}

static void
peas_extension_seed_class_init (PeasExtensionSeedClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionClass *extension_class = PEAS_EXTENSION_CLASS (klass);

  object_class->set_property = peas_extension_seed_set_property;
  object_class->constructed = peas_extension_seed_constructed;
  object_class->finalize = peas_extension_seed_finalize;

  extension_class->call = peas_extension_seed_call;

  g_object_class_install_property (object_class,
                                   PROP_JS_CONTEXT,
                                   g_param_spec_pointer ("js-context",
                                                         "JavaScript Context",
                                                         "A Seed JavaScript context",
                                                         G_PARAM_WRITABLE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_JS_OBJECT,
                                   g_param_spec_pointer ("js-object",
                                                         "JavaScript Object",
                                                         "A Seed JavaScript object",
                                                         G_PARAM_WRITABLE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

PeasExtension *
peas_extension_seed_new (GType       exten_type,
                         SeedContext js_context,
                         SeedObject  js_object)
{
  GType real_type;

  g_return_val_if_fail (js_context != NULL, NULL);
  g_return_val_if_fail (js_object != NULL, NULL);

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_SEED, exten_type);
  return PEAS_EXTENSION (g_object_new (real_type,
                                       "extension-type", exten_type,
                                       "js-context", js_context,
                                       "js-object", js_object,
                                       NULL));
}
