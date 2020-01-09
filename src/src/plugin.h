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

#ifndef _FH_PLUGIN_H_
#define _FH_PLUGIN_H_

extern GList *plugin_list;

typedef int (*fh_plugin_init_fn)(void *data);
typedef int (*fh_plugin_exit_fn)(void *data);

typedef struct fh_plugin {
    const char *name;
    fh_plugin_init_fn init_fn;
    fh_plugin_exit_fn exit_fn;
} fh_plugin_t;

void fh_plugin_init (void *plugin, void *data);
void fh_plugin_exit (void *plugin, void *data);

#endif /* _FH_PLUGIN_H_ */
