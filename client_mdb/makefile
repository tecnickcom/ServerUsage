
#==============================================================================+
# File name   : makefile
# Begin       : 2012-04-16
# Last Update : 2012-08-16
# Version     : 6.2.0
#
# Website     : https:#github.com/fubralimited/ServerUsage
#
# Description : Makefile to build ServerUsage-Client-MDB.
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
#				UK
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
CFLAGS=-O3 $(shell mysql_config --cflags)
CLIBS=$(shell mysql_config --libs)

# --- PATHS ---
PATHINSTALL=$(DESTDIR)/usr/bin/
PATHCONFIG=$(DESTDIR)/etc/
PATHMAN=$(DESTDIR)/usr/share/man/man8/

ifeq ($(wildcard /etc/rc.d/init.d),)
	PATHINIT=$(DESTDIR)/etc/init.d/
else
	PATHINIT=$(DESTDIR)/etc/rc.d/init.d/
endif

# --- TARGETS ---

# COMPILE THE TCP SENDER AND SYSTEMTAP KERNEL MODULE
all:
	cp -u ../client/serverusage_tcpsender.c ./ 2>/dev/null || :
	$(CC) $(CFLAGS) -o serverusage_tcpsender_mdb.bin serverusage_tcpsender.c
	$(CC) $(CFLAGS) -o serverusage_client_mdb.bin serverusage_client_mdb.c $(CLIBS)

# INSTALL ALL EXECUTABLES
install:
	mkdir -p $(PATHINSTALL)
	mkdir -p $(PATHCONFIG)
	mkdir -p $(PATHINIT)
	mkdir -p $(PATHMAN)
	install -m 0755 serverusage_tcpsender_mdb.bin $(PATHINSTALL)
	install -m 0755 serverusage_client_mdb.bin $(PATHINSTALL)
	install -m 0644 serverusage_client_mdb.conf $(PATHCONFIG)
	install -m 0755 serverusage_client_mdb $(PATHINIT)
	install -m 0644 serverusage_client_mdb.8 $(PATHMAN)
	gzip -f $(PATHMAN)serverusage_client_mdb.8

# REMOVE ALL INSTALLED FILES
uninstall:
	$(PATHINIT)serverusage_client_mdb stop
	chkconfig serverusage_client_mdb off
	rm -f $(PATHINSTALL)serverusage_tcpsender_mdb.bin
	rm -f $(PATHINSTALL)serverusage_client_mdb.bin
	rm -f $(PATHCONFIG)serverusage_client_mdb.conf
	rm -f $(PATHINIT)serverusage_client_mdb
	rm -f $(PATHMAN)serverusage_client_mdb.8.gz

# REMOVE BINARY AND EXECUTABLE FILES
clean:
	rm -f serverusage_tcpsender_mdb.bin
	rm -f serverusage_client_mdb.bin

#==============================================================================+
# END OF FILE
#==============================================================================+
