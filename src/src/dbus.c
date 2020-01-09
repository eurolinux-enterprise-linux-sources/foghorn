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

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "dbus.h"
#include "plugin.h"

static DBusConnection *bus = NULL;

static void
fh_dbus_disconnected (DBusMessage *msg)
{
    g_warning ("%s", dbus_message_get_member (msg));

    g_timeout_add_seconds (10, fh_dbus_init, NULL);
}

static void
fh_dbus_name_acquired (DBusMessage *msg)
{
    DBusError error;
    gchar *name;

    dbus_error_init (&error);

    dbus_message_get_args (msg, &error,
                           DBUS_TYPE_STRING, &name,
                           DBUS_TYPE_INVALID);

    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_message_get_args (%s)", error.message);
        dbus_error_free (&error);
        return;
    }

    g_message ("%s (%s)", dbus_message_get_member (msg), name);
}

static void
fh_dbus_name_lost (DBusMessage *msg)
{
    DBusError error;
    gchar *name;

    dbus_error_init (&error);

    dbus_message_get_args (msg, &error,
                           DBUS_TYPE_STRING, &name,
                           DBUS_TYPE_INVALID);

    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_message_get_args (%s)", error.message);
        dbus_error_free (&error);
        return;
    }

    g_message ("%s (%s)", dbus_message_get_member (msg), name);
}

static DBusHandlerResult
fh_dbus_filter (DBusConnection *conn, DBusMessage *msg, void *data)
{
    if (dbus_message_is_signal (msg, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        fh_dbus_disconnected (msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    if (dbus_message_is_signal (msg, DBUS_INTERFACE_DBUS, "NameAcquired")) {
        fh_dbus_name_acquired (msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    if (dbus_message_is_signal (msg, DBUS_INTERFACE_DBUS, "NameLost")) {
        fh_dbus_name_lost (msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

int
fh_dbus_init (void *data)
{
    DBusError error;
    gint reply;

    dbus_error_init (&error);

    bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_bus_get (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }

    g_assert (bus != NULL);

    reply = dbus_bus_request_name (bus, FH_DBUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_bus_request_name (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }
    if (reply != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_warning ("dbus_bus_request_name");
        return (1);
    }

    if (!dbus_connection_add_filter (bus, fh_dbus_filter, NULL, NULL)) {
        g_warning ("dbus_connection_add_filter");
        return (1);
    }

    dbus_connection_set_exit_on_disconnect (bus, FALSE);
    dbus_connection_setup_with_g_main (bus, NULL);

    g_list_foreach (plugin_list, fh_plugin_init, bus);

    return (0);
}

int
fh_dbus_exit (void *data)
{
    DBusError error;
    gint reply;

    dbus_error_init (&error);

    dbus_connection_remove_filter (bus, fh_dbus_filter, NULL);

    reply = dbus_bus_release_name (bus, FH_DBUS_NAME, &error);
    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_bus_release_name (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }
    if (reply != DBUS_RELEASE_NAME_REPLY_RELEASED) {
        g_warning ("dbus_bus_release_name");
        return (1);
    }

    g_list_foreach (plugin_list, fh_plugin_exit, bus);

    return (0);
}
