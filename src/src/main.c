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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>

#include "dbus.h"
#include "snmp.h"

GMainLoop *loop = NULL;
GLogLevelFlags flags = 0;

static gboolean debug = FALSE;
static gboolean verbose = FALSE;

static GOptionEntry options[] =
{
    { "debug",   'd', 0, G_OPTION_ARG_NONE, &debug,   "debug"   },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose" },
    { NULL }
};

static void
fh_signal_handler (int num)
{
    g_main_loop_quit (loop);
}

static int
fh_check_pidfile (const gchar *pidfile)
{
    return (0);
}

static int
fh_write_pidfile (const gchar *pidfile)
{
    return (0);
}

static void
fh_log_stderr (const gchar *log_domain, GLogLevelFlags log_level,
               const gchar *log_message, gpointer user_data)
{
    if ((log_level & flags) == 0) {
        return;
    }

    fprintf (stderr, "%s\n", log_message);
}

static void
fh_log_stdout (const gchar *log_domain, GLogLevelFlags log_level,
               const gchar *log_message, gpointer user_data)
{
    if ((log_level & flags) == 0) {
        return;
    }

    fprintf (stdout, "%s\n", log_message);
}

static void
fh_log_syslog (const gchar *log_domain, GLogLevelFlags log_level,
               const gchar *log_message, gpointer user_data)
{
    int priority;

    if ((log_level & flags) == 0) {
        return;
    }

    switch (G_LOG_LEVEL_MASK & log_level) {
    case G_LOG_LEVEL_ERROR:
        priority = LOG_ERR;
        break;
    case G_LOG_LEVEL_CRITICAL:
        priority = LOG_CRIT;
        break;
    case G_LOG_LEVEL_WARNING:
        priority = LOG_WARNING;
        break;
    case G_LOG_LEVEL_MESSAGE:
        priority = LOG_NOTICE;
        break;
    case G_LOG_LEVEL_DEBUG:
        priority = LOG_DEBUG;
        break;
    case G_LOG_LEVEL_INFO:
        priority = LOG_INFO;
        break;
    default:
        priority = LOG_INFO;
        break;
    }

    syslog (priority, "%s", log_message);
}

int
main (int argc, char **argv)
{
    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new (NULL);

    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, &error);
    g_option_context_free (context);

    if (error != NULL) {
        g_printerr ("error: %s\n", error->message);
        g_error_free (error);
        exit (EXIT_FAILURE);
    }

    if (verbose) {
        flags = (G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION | G_LOG_LEVEL_MASK);
    } else {
        flags = (G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION | G_LOG_LEVEL_WARNING);
    }

    if (debug) {
        g_log_set_default_handler (fh_log_stderr, NULL);
    } else {
        /* openlog ("foghorn", LOG_PID, LOG_DAEMON); */
        g_log_set_default_handler (fh_log_syslog, NULL);
    }

    loop = g_main_loop_new (NULL, FALSE);

    if (fh_dbus_init (NULL)) {
        g_printerr ("error: Failed to initialize dbus\n");
        exit (EXIT_FAILURE);
    }

    if (fh_snmp_init (NULL)) {
        g_printerr ("error: Failed to initialize snmp\n");
        exit (EXIT_FAILURE);
    }

    if (!debug && daemon (0, 0)) {
        g_printerr ("error: Failed to daemonize (%s)", g_strerror (errno));
        exit (EXIT_FAILURE);
    }

    signal (SIGINT, fh_signal_handler);

    g_warning ("foghorn %s started", VERSION);

    g_main_loop_run (loop);

    g_warning ("foghorn stopped");

    fh_dbus_exit (NULL);
    fh_snmp_exit (NULL);

    return (0);
}
