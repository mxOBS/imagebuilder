#!/bin/bash

# source kiwi settings
test -f /.kconfig && . /.kconfig
test -f /.profile && . /.profile

# initialize command-not-found database
apt-file update
update-command-not-found

# spawn getty on serial console
baseService serial-getty@ttymxc0.service on

# enable watchdog
sed -E -i "s;^#?RuntimeWatchdogSec=.*$;RuntimeWatchdogSec=60;g" "$buildroot/etc/systemd/system.conf"
