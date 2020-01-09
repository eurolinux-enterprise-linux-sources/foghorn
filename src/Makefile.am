SUBDIRS = src

man_MANS = man/foghorn.8

EXTRA_DIST = init/foghorn \
             dbus/dbus-foghorn.conf \
             snmp/REDHAT-FENCE-MIB \
             snmp/REDHAT-FOGHORN-MIB \
             snmp/REDHAT-RGMANAGER-MIB \
             $(man_MANS)

dbusconfdir = ${sysconfdir}/dbus-1/system.d/
dbusconf_DATA = dbus/dbus-foghorn.conf

snmpmibdir = ${datarootdir}/snmp/mibs/
snmpmib_DATA = snmp/REDHAT-FENCE-MIB \
               snmp/REDHAT-FOGHORN-MIB \
               snmp/REDHAT-RGMANAGER-MIB

sysinitdir = ${sysconfdir}/rc.d/init.d/
sysinit_DATA = init/foghorn
