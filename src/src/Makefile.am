sbin_PROGRAMS = foghorn

foghorn_SOURCES = \
	main.c main.h \
	dbus.c dbus.h \
	snmp.c snmp.h \
	plugin.c plugin.h \
	fence.c fence.h \
	rgmanager.c rgmanager.h \
	corosync.c corosync.h

foghorn_CFLAGS = \
	$(SNMP_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(DBUS_GLIB_FLAGS)

foghorn_LDFLAGS = \
	$(SNMP_LIBS) \
	$(GLIB_LIBS) \
	$(DBUS_LIBS) \
	$(DBUS_GLIB_LIBS)
