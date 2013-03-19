
#==============================================================================+
# File name   : makefile
# Begin       : 2012-04-16
# Last Update : 2012-05-22
# Version     : 6.3.7
#
# Website     : https://github.com/fubralimited/ServerUsage
#
# Description : Makefile to build ServerUsage-Server.
#
# Author: Nicola Asuni
#
# (c) Copyright:
#               Fubra Limited
#               Manor Coach House
#               Church Hill
#               Aldershot
#               Hampshire
#               GU12 4RQ
#               UK
#               http://www.fubra.com
#               support@fubra.com
#
# License:
#    Copyright (C) 2012-2012 Fubra Limited
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#    See LICENSE.TXT file for more information.
#==============================================================================+

# --- MACROS ---
CC=gcc
CFLAGS=-O3 -g -pipe -Wp,-D_THREAD_SAFE -D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector -fno-strict-aliasing -fwrapv -fPIC --param=ssp-buffer-size=4 -D_GNU_SOURCE
CLIBS=-lpthread -lsqlite3

# --- PATHS ---
PATHINSTALL=$(DESTDIR)/usr/bin/
PATHCONFIG=$(DESTDIR)/etc/
PATHSHARED=$(DESTDIR)/usr/share/serverusage/
PATHMAN=$(DESTDIR)/usr/share/man/man8/
PATHWEB=$(DESTDIR)/var/www/serverusage/
PATHDB=$(DESTDIR)/var/lib/serverusage/
ifeq ($(wildcard /etc/rc.d/init.d),)
	PATHINIT=$(DESTDIR)/etc/init.d/
else
	PATHINIT=$(DESTDIR)/etc/rc.d/init.d/
endif

# --- TARGETS ---

# COMPILE THE TCP RECEIVER
all:
	$(CC) $(CFLAGS) -o serverusage_tcpreceiver.bin serverusage_tcpreceiver.c $(CLIBS)

# INSTALL ALL EXECUTABLES AND SQL FILES
install:
	mkdir -p $(PATHCONFIG)
	mkdir -p $(PATHINSTALL)
	mkdir -p $(PATHINIT)
	mkdir -p $(PATHSHARED)
	mkdir -p $(PATHMAN)
	mkdir -p $(PATHWEB)
	mkdir -p $(PATHDB)
	install -m 0644 serverusage_server.conf $(PATHCONFIG)
	install -m 0755 serverusage_tcpreceiver.bin $(PATHINSTALL)
	install -m 0755 serverusage_dbagg.sh $(PATHINSTALL)
	install -m 0755 serverusage_server $(PATHINIT)
	install -m 0644 serverusage_database.sql $(PATHSHARED)
	install -m 0644 serverusage_server.8 $(PATHMAN)
	gzip -f $(PATHMAN)serverusage_server.8
	install -m 0755 serverusage_api.php $(PATHWEB)
	install -m 0755 serverusage_svg.html $(PATHWEB)
	@if test ! -f $(PATHDB)serverusage.db; then sqlite3 $(PATHDB)serverusage.db < $(PATHSHARED)serverusage_database.sql; fi

# REMOVE ALL INSTALLED FILES
uninstall:
	$(PATHINIT)serverusage_server stop
	chkconfig serverusage_server off
	rm -f $(PATHWEB)serverusage_svg.html
	rm -f $(PATHWEB)serverusage_api.php
	rmdir $(PATHWEB)
	rm -f $(PATHSHARED)serverusage_database.sql
	rmdir $(PATHSHARED)
	rm -f $(PATHMAN)serverusage_server.8.gz
	rm -f $(PATHINIT)serverusage_server
	rm -f $(PATHINSTALL)serverusage_dbagg.sh
	rm -f $(PATHINSTALL)serverusage_tcpreceiver.bin
	rm -f $(PATHCONFIG)serverusage_server.conf
	rm -f $(PATHDB)serverusage.db
	rmdir $(PATHDB)

# REMOVE COMPILED FILE
clean:
	rm -f serverusage_tcpreceiver.bin

#==============================================================================+
# END OF FILE
#==============================================================================+
