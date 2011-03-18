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

typedef struct {
  GITypeInfo type_info;
  gpointer ptr;
} OutArg;


gboolean seed_gvalue_from_seed_value (SeedContext    ctx,
                                      SeedValue      val,
                                      GType          type,
                                      GValue        *gval,
                                      SeedException *exception);

static void
peas_extension_seed_init (PeasExtensionSeed *sexten)
{
}

static gchar *
convert_property_name (const gchar *pname)
{
  gint i;
  gchar *prop_name;

  prop_name = g_strdup (pname);

  for (i = 0; prop_name[i] != '\0'; ++i)
    {
      if (prop_name[i] == '-')
        prop_name[i] = '_';
    }

  return prop_name;
}

static void
peas_extension_seed_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);
  SeedValue seed_value;
  SeedException exc = NULL;
  gchar *prop_name;

  /* Don't add properties as they could shadow the instance's */

  prop_name = convert_property_name (pspec->name);

  seed_value = seed_object_get_property (sexten->js_context, sexten->js_object,
                                         prop_name);

  g_free (prop_name);


  seed_gvalue_from_seed_value (sexten->js_context, seed_value,
                               pspec->value_type, value, &exc);

  if (exc != NULL)
    {
      gchar *exc_string;

      exc_string = seed_exception_to_string (sexten->js_context, exc);
      g_warning ("Seed Exception: %s", exc_string);

      g_free (exc_string);
    }
}

static void
peas_extension_seed_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);
  SeedValue seed_value;
  SeedException exc = NULL;
  gchar *prop_name;

  /* Don't add properties as they could shadow the instance's */

  seed_value = seed_value_from_gvalue (sexten->js_context,
                                       (GValue *) value, &exc);

  if (exc != NULL)
    {
      gchar *exc_string;

      exc_string = seed_exception_to_string (sexten->js_context, exc);
      g_warning ("Seed Exception: %s", exc_string);

      g_free (exc_string);
      return;
    }

  prop_name = convert_property_name (pspec->name);

  seed_object_set_property (sexten->js_context, sexten->js_object,
                            prop_name, seed_value);

  g_free (prop_name);
}

static void
peas_extension_seed_dispose (GObject *object)
{
  PeasExtensionSeed *sexten = PEAS_EXTENSION_SEED (object);

  if (sexten->js_object != NULL)
    {
      seed_value_unprotect (sexten->js_context, sexten->js_object);
      seed_context_unref (sexten->js_context);

      sexten->js_object = NULL;
      sexten->js_context = NULL;
    }
}

static SeedValue
get_argument (SeedContext    ctx,
              GIArgument    *arg,
              GITypeInfo    *arg_type_info,
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
    case GI_TYPE_TAG_UNICHAR:
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
  switch (g_type_info_get_tag (&arg->type_info))
    {
    case GI_TYPE_TAG_VOID:
      g_assert_not_reached ();
      break;
    case GI_TYPE_TAG_BOOLEAN:
      *((gboolean *) arg->ptr) = seed_value_to_boolean (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT8:
      *((gint8 *) arg->ptr) = seed_value_to_int (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT8:
      *((guint8 *) arg->ptr) = seed_value_to_uint (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT16:
      *((gint16 *) arg->ptr) = seed_value_to_int (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT16:
      *((guint16 *) arg->ptr) = seed_value_to_uint (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT32:
      *((gint32 *) arg->ptr) = seed_value_to_long (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UNICHAR:
    case GI_TYPE_TAG_UINT32:
      *((guint32 *) arg->ptr) = seed_value_to_ulong (ctx, value, exc);
      break;
    case GI_TYPE_TAG_INT64:
      *((gint64 *) arg->ptr) = seed_value_to_int64 (ctx, value, exc);
      break;
    case GI_TYPE_TAG_UINT64:
      *((guint64 *) arg->ptr) = seed_value_to_uint64 (ctx, value, exc);
      break;
    case GI_TYPE_TAG_FLOAT:
      *((gfloat *) arg->ptr) = seed_value_to_float (ctx, value, exc);
      break;
    case GI_TYPE_TAG_DOUBLE:
      *((gdouble *) arg->ptr) = seed_value_to_double (ctx, value, exc);
      break;

    case GI_TYPE_TAG_GTYPE:
      /* apparently, GType is meant to be a gsize, from gobject/gtype.h in glib */
      *((gsize *) arg->ptr) = seed_value_to_ulong (ctx, value, exc);
      break;

    case GI_TYPE_TAG_UTF8:
      *((gchar **) arg->ptr) = seed_value_to_string (ctx, value, exc);
      break;
    case GI_TYPE_TAG_FILENAME:
      *((gchar **) arg->ptr) = seed_value_to_filename (ctx, value, exc);
      break;

    case GI_TYPE_TAG_INTERFACE:
      *((gpointer *) arg->ptr) = seed_value_to_object (ctx, value, exc);
      break;

    /* FIXME */
    case GI_TYPE_TAG_ARRAY:
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_GHASH:
    case GI_TYPE_TAG_ERROR:
      *((gpointer *) arg->ptr) = NULL;

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
  gint n_args, i;
  SeedValue *js_in_args;
  OutArg *out_args;
  SeedValue js_ret, val;
  SeedException exc = NULL;
  gchar *exc_string;
  gint n_in_args = 0;
  gint n_out_args = 0;

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

  js_in_args = g_new0 (SeedValue, n_args);
  out_args = g_new0 (OutArg, n_args + 1);

  /* We put the return value first in the out tuple, as it seems to be
   * the common behaviour for GI-based bindings */
  g_callable_info_load_return_type (func_info, &out_args[0].type_info);
  if (g_type_info_get_tag (&out_args[0].type_info) != GI_TYPE_TAG_VOID)
    out_args[n_out_args++].ptr = &retval->v_pointer;

  /* Handle the other arguments */
  for (i = 0; i < n_args && exc == NULL; i++)
    {
      GIArgInfo arg_info;
      GIDirection direction;

      g_callable_info_load_arg (func_info, i, &arg_info);
      direction = g_arg_info_get_direction (&arg_info);
      g_arg_info_load_type (&arg_info, &out_args[n_out_args].type_info);

      if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT)
        {
          js_in_args[n_in_args++] = get_argument (sexten->js_context,
                                                  &args[i],
                                                  &out_args[n_out_args].type_info,
                                                  &exc);
        }

      if (direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT)
        out_args[n_out_args++].ptr = args[i].v_pointer;
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

  if (n_out_args == 1)
    {
      set_return_value (&out_args[0], sexten->js_context, js_ret, exc);
    }
  else if (n_out_args > 0 && seed_value_is_object (sexten->js_context, js_ret))
    {
      for (i = 0; i < n_out_args && exc == NULL; i++)
        {
          val = seed_object_get_property_at_index (sexten->js_context, js_ret,
                                                   i, exc);

          if (exc == NULL)
            set_return_value (&out_args[i], sexten->js_context, val, exc);
        }
    }

cleanup:

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
  object_class->get_property = peas_extension_seed_get_property;
  object_class->dispose = peas_extension_seed_dispose;

  extension_class->call = peas_extension_seed_call;
}

PeasExtension *
peas_extension_seed_new (GType       exten_type,
                         SeedContext js_context,
                         SeedObject  js_object)
{
  PeasExtensionSeed *sexten;
  GType real_type;

  g_return_val_if_fail (js_context != NULL, NULL);
  g_return_val_if_fail (js_object != NULL, NULL);

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_SEED, exten_type);
  sexten = PEAS_EXTENSION_SEED (g_object_new (real_type,
                                              "extension-type", exten_type,
                                              NULL));

  sexten->js_context = js_context;
  sexten->js_object = js_object;

  seed_context_ref (sexten->js_context);
  seed_value_protect (sexten->js_context, sexten->js_object);

  return PEAS_EXTENSION (sexten);
}
