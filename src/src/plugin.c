/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) Ryan O'Hara (rohara@redhat.com)
 * Copyright (c) Red Hat, Inc.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "plugin.h"

GList *plugin_list = NULL;

void
fh_plugin_init (void *data, void *user_data)
{
    fh_plugin_t *plugin = data;

    g_message ("fh_plugin_init (%s)", plugin->name);

    plugin->init_fn (user_data);
}

void
fh_plugin_exit (void *data, void *user_data)
{
    fh_plugin_t *plugin = data;

    g_message ("fh_plugin_exit (%s)", plugin->name);

    plugin->exit_fn (user_data);
}
