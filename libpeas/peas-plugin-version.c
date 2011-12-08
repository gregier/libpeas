/*
 * peas-plugin-version.c
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

#include "peas-plugin-version.h"

#define VERSION_STAR -1

typedef enum {
  VERSION_SECTION_MAJOR,
  VERSION_SECTION_MINOR,
  VERSION_SECTION_MICRO
} VersionSection;

struct _PeasPluginVersion {
  volatile gint refcount;

  gint major;
  gint minor;
  gint micro;
};

G_DEFINE_BOXED_TYPE (PeasPluginVersion, peas_plugin_version,
                     peas_plugin_version_ref,
                     peas_plugin_version_unref)

static gint
version_compare (const PeasPluginVersion *a,
                 const PeasPluginVersion *b)
{
  /* Major version is never VERSION_STAR */
  if (a->major == b->major &&
      (a->minor == VERSION_STAR || b->minor == VERSION_STAR ||
       (a->minor == b->minor &&
        (a->micro == VERSION_STAR || b->micro == VERSION_STAR))))
    return 0;

  if (a->major > b->major ||
      (a->major == b->major && (a->minor > b->minor ||
       (a->minor == b->minor && a->micro > b->micro))))
    return 1;

  if (a->major < b->major ||
      (a->major == b->major && (a->minor < b->minor ||
       (a->minor == b->minor && a->micro < b->micro))))
    return -1;

  return 0;
}

/**
 * peas_plugin_version_new:
 * @version_str: A version string.
 *
 * Creates a #PeasPluginVersion which represents the @version_str.
 * The format for @version_str is "$major.$minor.$micro" where
 * major, minor and micro are positive numbers. The minor or micro
 * number may also be a * which signifies any number.
 *
 * Returns: a new #PeasPluginVersion for @version_str.
 */
PeasPluginVersion *
peas_plugin_version_new (const gchar *version_str)
{
  gint i = 0;
  gint *version_number;
  PeasPluginVersion *version;
  VersionSection section = VERSION_SECTION_MAJOR;

  g_return_val_if_fail (version_str != NULL && *version_str != '\0', NULL);

  version = g_slice_new0 (PeasPluginVersion);
  version->refcount = 1;

  version_number = &version->major;

  for (; version_str[i] != '\0'; )
    {
      /* Must start with a number or * (dot not allowed) */
      if (!g_ascii_isdigit (version_str[i]) && version_str[i] != '*')
        {
          g_warning ("Invalid character '%c' in version string: '%s'",
                     version_str[i], version_str);
          goto error;
        }

      if (version_str[i] == '*')
        {
          if (section == VERSION_SECTION_MAJOR)
            {
              g_warning ("Cannot use star for major version");
              goto error;
            }

          if (*version_number != 0)
            {
              g_warning ("Cannot use star in number in version string: '%s'",
                         version_str);
              goto error;
            }

          if (version_str[++i] != '\0')
            {
              g_warning ("Star does not end version string: '%s'", version_str);
              goto error;
            }

          *version_number = VERSION_STAR;

          /* A * ends the version string */
          break;
        }

      *version_number *= 10;
      *version_number += g_ascii_digit_value (version_str[i]);

      if (version_str[++i] == '.')
        {
          ++i;
          ++section;

          /* No more than 2 dots */
          if (section > VERSION_SECTION_MICRO)
            {
              g_warning ("Too many dots in version string: '%s'", version_str);
              goto error;
            }

          /* Must have something after the dot */
          if (!g_ascii_isdigit (version_str[i]) && version_str[i] != '*')
            {
              g_warning ("Number missing after dot in version string: '%s'",
                         version_str);
              goto error;
            }

          if (section == VERSION_SECTION_MINOR)
            version_number = &version->minor;
          else
            version_number = &version->micro;
        }
    }

  /* 0.0.0 is not a version number, 0.0.1 is */
  if (version->major == 0 && version->minor == 0 && version->micro == 0)
    {
      g_warning ("Invalid version: '%s'", version_str);
      goto error;
    }

  return version;

error:

  g_slice_free (PeasPluginVersion, version);

  return NULL;
}

/**
 * peas_plugin_version_ref:
 * @version: A #PeasPluginVersion.
 *
 * Increases the reference count of @version.
 *
 * Returns: @version.
 */
PeasPluginVersion *
peas_plugin_version_ref (PeasPluginVersion *version)
{
  g_return_val_if_fail (version != NULL, NULL);

  g_atomic_int_inc (&version->refcount);

  return version;
}

/**
 * peas_plugin_version_unref:
 * @version: A #PeasPluginVersion.
 *
 * Decreases the reference count of @version.
 * When its reference count drops to 0, @version is freed.
 */
void
peas_plugin_version_unref (PeasPluginVersion *version)
{
  g_return_if_fail (version != NULL);

  if (!g_atomic_int_dec_and_test (&version->refcount))
    return;

  g_slice_free (PeasPluginVersion, version);
}

/**
 * peas_plugin_version_check:
 * @a: A #PeasPluginVersion.
 * @b: A #PeasPluginVersion.
 * @op: A #PeasPluginVersionOperation.
 *
 * Check that @lhs is @op to @rhs.
 *
 * Returns: %TRUE if @lhs is @op to @rhs, otherwise %FALSE.
 */
gboolean
peas_plugin_version_check (const PeasPluginVersion    *a,
                           const PeasPluginVersion    *b,
                           PeasPluginVersionOperation  op)
{
  gint comparison;

  g_return_val_if_fail (a != NULL, FALSE);
  g_return_val_if_fail (b != NULL, FALSE);

  if (op != PEAS_PLUGIN_VERSION_OPERATION_EQ &&
      op != PEAS_PLUGIN_VERSION_OPERATION_NE &&
      op != PEAS_PLUGIN_VERSION_OPERATION_GT &&
      op != PEAS_PLUGIN_VERSION_OPERATION_LT &&
      op != PEAS_PLUGIN_VERSION_OPERATION_GTE &&
      op != PEAS_PLUGIN_VERSION_OPERATION_LTE)
    g_warning ("Mixed operations are in use");

  comparison = version_compare (a, b);

  if ((op & PEAS_PLUGIN_VERSION_OPERATION_EQ && comparison == 0) ||
      (op & PEAS_PLUGIN_VERSION_OPERATION_NE && comparison != 0) ||
      (op & PEAS_PLUGIN_VERSION_OPERATION_GT && comparison  > 0) ||
      (op & PEAS_PLUGIN_VERSION_OPERATION_LT && comparison  < 0))
    return TRUE;

  return FALSE;
}

/**
 * peas_plugin_version_to_string:
 * @version: A #PeasPluginVersion.
 *
 * Converts @version into a string of the form "$major.$minor.$micro".
 *
 * Returns: A newly-allocated string representing @version.
 */
gchar *
peas_plugin_version_to_string (const PeasPluginVersion *version)
{
  g_return_val_if_fail (version != NULL, NULL);

  /* The major version is never VERSION_STAR */

  if (version->minor == VERSION_STAR)
    return g_strdup_printf ("%i.*", version->major);

  if (version->micro == VERSION_STAR)
    return g_strdup_printf ("%i.%i.*", version->major, version->minor);

  if (version->minor == 0 && version->micro == 0)
    return g_strdup_printf ("%i", version->major);

  if (version->micro == 0)
    return g_strdup_printf ("%i.%i", version->major, version->minor);

  return g_strdup_printf ("%i.%i.%i",
                          version->major, version->minor, version->micro);
}
