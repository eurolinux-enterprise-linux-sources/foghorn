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

#include "fence.h"
#include "plugin.h"

static int fh_fence_init (void *data);
static int fh_fence_exit (void *data);

static fh_plugin_t fence_plugin =
{
    .name = "fence",
    .init_fn = fh_fence_init,
    .exit_fn = fh_fence_exit,
};

static void
fh_fence_snmp_fence_node (const gchar *nodename, int nodeid, int result)
{
    netsnmp_variable_list *notification_vars = NULL;

    /* SNMPv2-MIB::snmpTrapOID.0 */
    oid objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    size_t objid_snmptrap_len = OID_LENGTH (objid_snmptrap);

    /* REDHAT-FENCE-MIB::fenceNotifyFenceNode */
    oid notification_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 10, 0, 1 };
    size_t notification_oid_len = OID_LENGTH (notification_oid);

    /* REDHAT-FENCE-MIB::fenceNodeName.0 */
    oid nodename_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 10, 1, 1, 0 };
    size_t nodename_oid_len = OID_LENGTH (nodename_oid);

    /* REDHAT-FENCE-MIB::fenceNodeID.0 */
    oid nodeid_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 10, 1, 2, 0 };
    size_t nodeid_oid_len = OID_LENGTH (nodeid_oid);

    /* REDHAT-FENCE-MIB::fenceResult.0 */
    oid result_oid[] = { 1, 3, 6, 1, 4, 1, 2312, 10, 1, 3, 0 };
    size_t result_oid_len = OID_LENGTH (result_oid);

    snmp_varlist_add_variable (&notification_vars,
                               objid_snmptrap, objid_snmptrap_len,
                               ASN_OBJECT_ID,
                               (u_char *) notification_oid,
                               notification_oid_len * sizeof (oid));

    snmp_varlist_add_variable (&notification_vars,
                               nodename_oid, nodename_oid_len,
                               ASN_OCTET_STR,
                               nodename, strlen (nodename));

    snmp_varlist_add_variable (&notification_vars,
                               nodeid_oid, nodeid_oid_len,
                               ASN_INTEGER,
                               (u_char *) &nodeid, sizeof (nodeid));

    snmp_varlist_add_variable (&notification_vars,
                               result_oid, result_oid_len,
                               ASN_INTEGER,
                               (u_char *) &result, sizeof (result));

    send_v2trap (notification_vars);
    snmp_free_varbind (notification_vars);
}

static void
fh_fence_dbus_fence_node (DBusMessage *msg)
{
    DBusError error;
    gchar *nodename;
    gint nodeid;
    gint result;

    dbus_error_init (&error);

    dbus_message_get_args (msg, &error,
                           DBUS_TYPE_STRING, &nodename,
                           DBUS_TYPE_INT32, &nodeid,
                           DBUS_TYPE_INT32, &result,
                           DBUS_TYPE_INVALID);

    if (dbus_error_is_set (&error)) {
        g_warning ("dbus_message_get_args (%s)", error.message);
        return;
    }

    fh_fence_snmp_fence_node (nodename, nodeid, result);
}

static DBusHandlerResult
fh_fence_filter (DBusConnection *conn, DBusMessage *msg, void *data)
{
    if (dbus_message_is_signal (msg, FH_FENCE_INTERFACE, "FenceNode")) {
        fh_fence_dbus_fence_node (msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

static int
fh_fence_init (void *data)
{
    DBusConnection *bus = data;
    DBusError error;

    dbus_error_init (&error);

    dbus_bus_add_match (bus, "type='signal',interface='"FH_FENCE_INTERFACE"'", &error);
    if (dbus_error_is_set (&error)) {
        plugin_list = g_list_remove (plugin_list, &fence_plugin);
        g_warning ("dbus_bus_add_match (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }

    if (!dbus_connection_add_filter (bus, fh_fence_filter, NULL, NULL)) {
        plugin_list = g_list_remove (plugin_list, &fence_plugin);
        g_warning ("dbus_connection_add_filter");
        return (1);
    }

    return (0);
}

static int
fh_fence_exit (void *data)
{
    DBusConnection *bus = data;
    DBusError error;

    dbus_error_init (&error);

    dbus_bus_remove_match (bus, "type='signal',interface='"FH_FENCE_INTERFACE"'", &error);
    if (dbus_error_is_set (&error)) {
        plugin_list = g_list_remove (plugin_list, &fence_plugin);
        g_warning ("dbus_bus_remove_match (%s)", error.message);
        dbus_error_free (&error);
        return (1);
    }

    dbus_connection_remove_filter (bus, fh_fence_filter, NULL);

    plugin_list = g_list_remove (plugin_list, &fence_plugin);

    return (0);
}

static void __attribute__ ((constructor))
fh_fence_register (void)
{
    plugin_list = g_list_append (plugin_list, &fence_plugin);
}
