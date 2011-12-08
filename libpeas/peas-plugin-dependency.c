/*
 * peas-plugin-dependency.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
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

#include <glib.h>

#include "peas-plugin-dependency.h"

typedef enum {
  DEP_TYPE_ANY,
  DEP_TYPE_SINGLE,
  DEP_TYPE_RANGE
} DepTypes;

struct _PeasPluginDependency {
  volatile gint refcount;

  gchar *name;

  DepTypes dep_type;
  union {
    struct {
      PeasPluginVersion *version;
      PeasPluginVersionOperation op;
    } single;

    struct {
      PeasPluginVersion *low;
      PeasPluginVersion *high;
    } range;
  } data;
};

G_DEFINE_BOXED_TYPE (PeasPluginDependency, peas_plugin_dependency,
                     peas_plugin_dependency_ref,
                     peas_plugin_dependency_unref)

/**
 * peas_plugin_dependency_new:
 *
 * There are three formats for @dep_str. The first is just a
 * name which any version will satisfy. The second has the form
 * "$name $operation $version" where $operation can be
 * ==, !=, >, <, >= or <= and $version is in the form accepted by
 * peas_plugin_version_new(). The final format is a range which has
 * the form "$name $version-$version" and uses the same formatting
 * for version as the second form.
 *
 * Returns: A new #PeasPluginDependency.
 */
PeasPluginDependency *
peas_plugin_dependency_new (const gchar *dep_str)
{
  gint i = 0;
  PeasPluginDependency *dep;

  g_return_val_if_fail (dep_str != NULL && *dep_str != '\0', NULL);

  dep = g_slice_new (PeasPluginDependency);
  dep->refcount = 1;
  dep->name = NULL;

  /* Only a-z, A-Z, 0-9, - and _ are module names */
  while (g_ascii_isalnum (dep_str[i]) || dep_str[i] == '-' || dep_str[i] == '_')
    ++i;

  if (i == 0)
    {
      g_warning ("Invalid character '%c' in dependency string: '%s'",
                 dep_str[i], dep_str);
      goto error;
    }

  dep->name = g_strndup (dep_str, i);

  /* If we are only a name */
  if (dep_str[i] == '\0')
    {
      dep->dep_type = DEP_TYPE_ANY;
      return dep;
    }

  if (dep_str[i] != ' ')
    {
      g_warning ("Invalid character '%c' in dependency string: '%s'",
                 dep_str[i], dep_str);
      goto error;
    }

  if (g_ascii_isdigit (dep_str[++i]))
    {
      gint j = i;
      gchar *str;

      dep->dep_type = DEP_TYPE_RANGE;

      while (dep_str[j] != '-' && dep_str[j] != '\0')
        ++j;

      str = g_strndup (dep_str + i, j - i);
      dep->data.range.low = peas_plugin_version_new (str);
      g_free (str);

      if (dep->data.range.low == NULL)
         goto error;

      dep->data.range.high = peas_plugin_version_new (dep_str + j + 1);

      if (dep->data.range.high == NULL)
        {
          peas_plugin_version_unref (dep->data.range.low);
          goto error;
        }

      /* Require that low < high */
      if (peas_plugin_version_check (dep->data.range.low,
                                     dep->data.range.high,
                                     PEAS_PLUGIN_VERSION_OPERATION_LT))
        return dep;

      g_warning ("Invalid version range in dependency string: '%s'", dep_str);
      peas_plugin_version_unref (dep->data.range.low);
      peas_plugin_version_unref (dep->data.range.high);
      goto error;
    }
  else
    {
      dep->dep_type = DEP_TYPE_SINGLE;

      /* Get the operation */
      switch (dep_str[i])
        {
        case '=':
          if (dep_str[i + 1] != '=')
            {
              g_warning ("Missing '=' after '=' in dependency string: '%s'",
                         dep_str);
              goto error;
            }

          dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_EQ;
          break;
        case '!':
          if (dep_str[i + 1] != '=')
            {
              g_warning ("Missing '=' after '!' in dependency string: '%s'",
                         dep_str);
              goto error;
            }

          dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_NE;
          break;
        case '>':
          if (dep_str[i + 1] != '=')
            dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_GT;
          else
            dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_GTE;

          break;
        case '<':
          if (dep_str[i + 1] != '=')
            dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_LT;
          else
            dep->data.single.op = PEAS_PLUGIN_VERSION_OPERATION_LTE;

          break;
        default:
          g_warning ("Invalid operator '%c' in dependency string: '%s'",
                     dep_str[i], dep_str);
          goto error;
        }

      /* Move past the operation */
      if (dep_str[i + 1] != '=')
        i += 1;
      else
        i += 2;

      if (dep_str[i++] != ' ')
        {
          g_warning ("Missing space after operator in dependency string: '%s'",
                     dep_str);
          goto error;
        }

      dep->data.single.version = peas_plugin_version_new (dep_str + i);

      if (dep->data.single.version != NULL)
        return dep;
    }

  /* Version warning already emitted */

error:

  if (dep->name != NULL)
    g_free (dep->name);

  g_slice_free (PeasPluginDependency, dep);

  return NULL;
}

/**
 * peas_plugin_dependency_ref:
 * @dep: A #PeasPluginDependency.
 *
 * Increases the reference count of @dep.
 *
 * Returns: @dep.
 */
PeasPluginDependency *
peas_plugin_dependency_ref (PeasPluginDependency *dep)
{
  g_return_val_if_fail (dep != NULL, NULL);

  g_atomic_int_inc (&dep->refcount);

  return dep;
}

/**
 * peas_plugin_dependency_unref:
 * @dep: A #PeasPluginDependency.
 *
 * Decreases the reference count of @dep.
 * When its reference count drops to 0, @dep is freed.
 */
void
peas_plugin_dependency_unref (PeasPluginDependency *dep)
{
  g_return_if_fail (dep != NULL);

  if (!g_atomic_int_dec_and_test (&dep->refcount))
    return;

  g_free (dep->name);

  switch (dep->dep_type)
    {
    case DEP_TYPE_ANY:
      break;
    case DEP_TYPE_SINGLE:
      peas_plugin_version_unref (dep->data.single.version);
      break;
    case DEP_TYPE_RANGE:
      peas_plugin_version_unref (dep->data.range.low);
      peas_plugin_version_unref (dep->data.range.high);
      break;
    default:
      g_assert_not_reached ();
    }

  g_slice_free (PeasPluginDependency, dep);
}

/**
 * peas_plugin_dependency_check:
 * @dep: A #PeasPluginDependency.
 * @version_str: (allow-none): A version string,
 *  see peas_plugin_version_new(), or %NULL.
 *
 * Returns if @version_str satisfies the dependency of @dep.
 *
 * Note: if @dep is an any version dependency or @version_str
 * is %NULL then @version_str will not be parsed.
 *
 * Returns: %TRUE if @version_str satisfies the dependency of
 * @dep, otherwise %FALSE.
 */
gboolean
peas_plugin_dependency_check (const PeasPluginDependency *dep,
                              const gchar                *version_str)
{
  gboolean retval;
  PeasPluginVersion *version;

  g_return_val_if_fail (dep != NULL, FALSE);

  if (dep->dep_type == DEP_TYPE_ANY)
    return TRUE;

  if (version_str == NULL)
    return FALSE;

  version = peas_plugin_version_new (version_str);

  /* Version warning already emitted */
  if (version == NULL)
    return FALSE;

  retval = peas_plugin_dependency_check_version (dep, version);

  peas_plugin_version_unref (version);

  return retval;
}

/**
 * peas_plugin_dependency_check_version:
 * @dep: A #PeasPluginDependency.
 * @version: (allow-none): A #PeasPluginVersion, or %NULL.
 *
 * Returns if @version satisfies the dependency of @dep.
 *
 * Note: if @dep is an any version dependency or @version
 * is %NULL then @version will not be checked.
 *
 * Returns: %TRUE if @version satisfies the dependency of
 * @dep, otherwise %FALSE.
 */
gboolean
peas_plugin_dependency_check_version (const PeasPluginDependency *dep,
                                      const PeasPluginVersion    *version)
{
  g_return_val_if_fail (dep != NULL, FALSE);

  /* This may seem backwards, however if we have
   * "name < 1.2" and the version is "1.1" then we
   * want to check that version "1.1" < "1.2" not "1.2" < "1.1"
   */

  if (dep->dep_type == DEP_TYPE_ANY)
    return TRUE;

  if (version == NULL)
    return FALSE;

  switch (dep->dep_type)
    {
    case DEP_TYPE_SINGLE:
      return peas_plugin_version_check (version, dep->data.single.version,
                                        dep->data.single.op);
    case DEP_TYPE_RANGE:
      return peas_plugin_version_check (version, dep->data.range.low,
                                        PEAS_PLUGIN_VERSION_OPERATION_GTE) &&
             peas_plugin_version_check (version, dep->data.range.high,
                                        PEAS_PLUGIN_VERSION_OPERATION_LTE);
    default:
      g_assert_not_reached ();
    }
}

/**
 * peas_plugin_dependency_get_name:
 * @dep: A #PeasPluginDependency.
 *
 * Returns the name of the dependency of @dep.
 *
 * Returns: the dependency's name.
 */
const gchar *
peas_plugin_dependency_get_name (const PeasPluginDependency *dep)
{
  g_return_val_if_fail (dep != NULL, NULL);

  return dep->name;
}

/**
 * peas_plugin_dependency_to_string:
 * @dep: A #PeasPluginDependency.
 *
 * Converts @dep into a string.
 *
 * Returns: A newly-allocated string representing @dep.
 */
gchar *
peas_plugin_dependency_to_string (const PeasPluginDependency *dep)
{
  gchar *str;

  g_return_val_if_fail (dep != NULL, NULL);

  if (dep->dep_type == DEP_TYPE_ANY)
    {
      str = g_strdup (dep->name);
    }
  else if (dep->dep_type == DEP_TYPE_SINGLE)
    {
      gchar *version;
      const gchar *op;

      version = peas_plugin_version_to_string (dep->data.single.version);

      switch (dep->data.single.op)
        {
        case PEAS_PLUGIN_VERSION_OPERATION_EQ:
          op = "==";
          break;
        case PEAS_PLUGIN_VERSION_OPERATION_NE:
          op = "!=";
          break;
        case PEAS_PLUGIN_VERSION_OPERATION_LT:
          op = "<";
          break;
        case PEAS_PLUGIN_VERSION_OPERATION_GT:
          op = ">";
          break;
        case PEAS_PLUGIN_VERSION_OPERATION_LTE:
          op = "<=";
          break;
        case PEAS_PLUGIN_VERSION_OPERATION_GTE:
          op = ">=";
          break;
        default:
          g_assert_not_reached ();
        }

      str = g_strdup_printf ("%s %s %s", dep->name, op, version);

      g_free (version);
    }
  else if (dep->dep_type == DEP_TYPE_RANGE)
    {
      gchar *low;
      gchar *high;

      low = peas_plugin_version_to_string (dep->data.range.low);
      high = peas_plugin_version_to_string (dep->data.range.high);

      str = g_strdup_printf ("%s %s-%s", dep->name, low, high);

      g_free (high);
      g_free (low);
    }
  else
    {
      g_assert_not_reached ();
    }

  return str;
}
