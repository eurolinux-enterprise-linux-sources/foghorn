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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "rgmanager.h"
#include "plugin.h"

static int fh_rgmanager_init (void *data);
static int fh_rgmanager_exit (void *data);

static fh_plugin_t rgmanager_plugin =
{
    .name = "rgmanager",
    .init_fn = fh_rgmanager_init,
    .exit_fn = fh_rgmanager_exit,
};

static void
fh_snmp_service_state_change (const gchar *name, const gchar *state, const gchar *flags,
                              const gchar *current, const gchar *previous)
{
    netsnmp_variable_list *notification_vars = NULL;

    /* SNMPv2-MIB::snmpTrapOID.0 */
    oid objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    size_t objid_snmptrap_len = OID_LENGTH (objid_snmptrap);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServiceStateChange */
    oid notification_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 0, 1 };
    size_t notification_oid_len = OID_LENGTH (notification_oid);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServiceName.0 */
    oid name_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 1, 1, 0 };
    size_t name_oid_len = OID_LENGTH (name_oid);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServiceState.0 */
    oid state_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 1, 2, 0 };
    size_t state_oid_len = OID_LENGTH (state_oid);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServiceFlags.0 */
    oid flags_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 1, 3, 0 };
    size_t flags_oid_len = OID_LENGTH (flags_oid);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServiceCurrentOwner.0 */
    oid current_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 1, 4, 0 };
    size_t current_oid_len = OID_LENGTH (current_oid);

    /* REDHAT-RGMANAGER-MIB::rgmanagerServicePreviousOwner.0 */
    oid previous_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 11, 1, 5, 0 };
    size_t previous_oid_len = OID_LENGTH (previous_oid);

    snmp_varlist_add_variable (&notification_vars,
                               objid_snmptrap, objid_snmptrap_len,
                               ASN_OBJECT_ID,
                               (u_char *) notification_oid,
                               notification_oid_len * sizeof (oid));

    snmp_varlist_add_variable (&notification_vars,
                               name_oid, name_oid_len,
                               ASN_OCTET_STR,
                               name, strlen (name));

    snmp_varlist_add_variable (&notification_vars,
                               state_oid, state_oid_len,
                               ASN_OCTET_STR,
                               state, strlen (state));

    snmp_varlist_add_variable (&notification_vars,
                               flags_oid, flags_oid_len,
                               ASN_OCTET_STR,
                               flags, strlen (flags));

    snmp_varlist_add_variable (&notification_vars,
                               current_oid, current_oid_len,
                               ASN_OCTET_STR,
                               current, strlen (current));

    snmp_varlist_add_variable (&notification_vars,
                               previous_oid, previous_oid_len,
                               ASN_OCTET_STR,
                               previous, strlen (previous));

    send_v2trap (notification_vars);
    snmp_free_varbind (notification_vars);
}

static void
fh_dbus_service_state_change (DBusMessage *msg)
{
    DBusError error;
    gchar *name;
    gchar *state;
    gchar *flags;
    gchar *current;
    gchar *previous;

    dbus_error_init (&error);

    dbus_message_get_args (msg, &error,
                           DBUS_TYPE_STRING, &name,
                           DBUS_TYPE_STRING, &state,
                           DBUS_TYPE_STRING, &flags,
                           DBUS_TYPE_STRING, &current,
                           DBUS_TYPE_STRING, &previous,
                           DBUS_TYPE_INVALID);

    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_message_get_args (%s)", error.message);
        return;
    }

    fh_snmp_service_state_change (name, state, flags, current, previous);
}

static DBusHandlerResult
fh_rgmanager_filter (DBusConnection *conn, DBusMessage *msg, void *data)
{
    if (dbus_message_is_signal (msg, FH_RGMANAGER_INTERFACE, "ServiceStateChange")) {
        fh_dbus_service_state_change (msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

static int
fh_rgmanager_init (void *data)
{
    DBusConnection *bus = data;
    DBusError error;

    dbus_error_init (&error);

    dbus_bus_add_match (bus, "type='signal',interface='"FH_RGMANAGER_INTERFACE"'", &error);
    if (dbus_error_is_set (&error)) {
        plugin_list = g_list_remove (plugin_list, &rgmanager_plugin);
        g_warning ("dbus_bus_add_match (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }

    if (!dbus_connection_add_filter (bus, fh_rgmanager_filter, NULL, NULL)) {
        plugin_list = g_list_remove (plugin_list, &rgmanager_plugin);
        g_warning ("dbus_connection_add_filter");
        return (1);
    }

    return (0);
}

static int
fh_rgmanager_exit (void *data)
{
    DBusConnection *bus = data;
    DBusError error;

    dbus_error_init (&error);

    dbus_bus_remove_match (bus, "type='signal',interface='"FH_RGMANAGER_INTERFACE"'", &error);
    if (dbus_error_is_set (&error)) {
        plugin_list = g_list_remove (plugin_list, &rgmanager_plugin);
        g_warning ("dbus_bus_remove_match (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }

    dbus_connection_remove_filter (bus, fh_rgmanager_filter, NULL);

    plugin_list = g_list_remove (plugin_list, &rgmanager_plugin);

    return (0);
}

static void __attribute__ ((constructor))
fh_rgmanager_register (void)
{
    plugin_list = g_list_append (plugin_list, &rgmanager_plugin);
}
