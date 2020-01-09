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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmp.h"

int
fh_snmp_read (void *data)
{
    agent_check_and_process (0);

    return (1);
}

int
fh_snmp_init (void *data)
{
    snmp_enable_syslog ();

    netsnmp_ds_set_boolean (NETSNMP_DS_APPLICATION_ID,
                            NETSNMP_DS_AGENT_ROLE, 1);

    init_agent (FH_SNMP_NAME);
    init_snmp (FH_SNMP_NAME);

    g_timeout_add_seconds (1, fh_snmp_read, NULL);

    snmp_disable_log ();

    return (0);
}

int
fh_snmp_exit (void *data)
{
    snmp_shutdown (FH_SNMP_NAME);

    return (0);
}
