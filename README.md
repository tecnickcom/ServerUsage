ServerUsage - README
====================

+ Name: ServerUsage

+ Version: 6.3.4

+ Release date: 2012-11-15

+ Author: Nicola Asuni

+ Copyright (2012-2012):

> > Fubra Limited  
> > Manor Coach House  
> > Church Hill  
> > Aldershot  
> > Hampshire  
> > GU12 4RQ  
> > <http://www.fubra.com>  
> > <support@fubra.com>  


SOFTWARE LICENSE:
-----------------

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

See LICENSE.TXT file for more information.


DESCRIPTION:
------------

The ServerUsage system is designed to collect and process statistic information from computers running a GNU-Linux Operating System.

This project is composed by:


## ServerUsage-Server ##

The ServerUsage-Server program listen on a TCP port for incoming log data (from ServerUsage-Client) and store them on a SQLite table. An script (serverusage_dbagg.sh) is executed periodically to aggregate data on another table and delete obsolete data.

The serverusage_api.php script can be remotely used to extract formatted data from the database or display graphs.


## ServerUsage-Client ##

The ServerUsage-Client program is composed by two main sections: the SystemTap (http://sourceware.org/systemtap) serverusage_client.ko kernel module to collect and output usage statistics of the machine where it is installed, and the serverusage_tcpsender.bin to send the output to a log server via TCP.


## ServerUsage-Client-MDB ##

The ServerUsage-Client-MDB program is used to collect user statistics form a MariaDB database.



## GENERAL USAGE SCHEMA ##
	
	(ServerUsage-Client) - - TCP CONNECTION - -> (ServerUsage-Server)

	([serverusage_client.ko]==>[serverusage_tcpsender.bin]) - - TCP CONNECTION - -> ([serverusage_tcpreceiver]==>[SQLite log_raw table (raw data)])

	[SQLite log_raw table]==>[serverusage_dbagg.sh]==>[SQLite log_agg_hst table (aggregated data)]

	[SQLite log_agg_hst table]==>[serverusage_api.php]==>[Data with selected format or SVG graph]

## API ##

The serverusage_api.php script can be remotely used to extract formatted data from the database or display graphs.

### PARAMETERS: ###

	from: (integer) starting timestamp in seconds since EPOCH;
	to: (integer) starting timestamp in seconds since EPOCH;
	metric: (not available with svg mode) type of info to extract; Possible values are: 'uid', 'ip', 'uip', 'grp', 'glb', 'all'. The return values for each metric are:
		uid : user_id, cpu_ticks;
		uidt : user_id, cpu_ticks, minimum start time, maximum end time;
		ip  : ip, net_in, net_out;
		ipt  : ip, net_in, net_out, minimum start time, maximum end time;
		uip : user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out;
		uipt : user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out, minimum start time, maximum end time;
		grp : start_time, end_time, user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out;
		glb : (default for SVG mode) lah_start_time, lah_end_time, lah_cpu_ticks, lah_io_read, lah_io_write, lah_netin, lah_netout;
		all : start_time, end_time, process, user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out.
	uid : (integer) if set, filter result for the requested user ID;
	ip : (IP address) if set, filter result for the requested IP address.
	mode: output format ('json' = JSON, 'csv' = CSV TAB-Separated Values, 'psa' = base64 encoded PHP Serialized array, 'svg' = SVG).

	ADDITIONAL PARAMETERS FOR SVG MODE:

	width: (integer) optional width for SVG output (default 1024; minimum 50).
	height: (integer) optional height for SVG output; will be rounded to a multiple of 5 (default 750, minimum 50).
	scale: linear = vertical linear scale (default), log = vertical logarithmic scale.
	bgcol: type of background color: 'dark' or 'light' (default). Note: the SVG background is always transparent.
	gtype: sequence of number representing the graphs to display: 1 = CPU TICKS, 2 = IO READ, 3 = IO WRITE, 4 = NET IN, 5 = NET OUT. Default: 12345.

### USAGE EXAMPLES: ###

	JSON:
		serverusage_api.php?from=1332769800&to=1332845100&metric=uid&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=uidt&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=ip&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=ipt&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=uip&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=uipt&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=json

	CSV:
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=csv

	BASE64 ENCODED PHP SERIALIZED ARRAY:
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=psa

	SVG:
		serverusage_api.php?from=1332769800&to=1332845100&mode=svg&width=1024&height=750&scale=log
		serverusage_api.php?from=1333532663&to=1333627917&mode=svg&scale=log&bgcol=light&gtype=12345
		serverusage_api.php?from=1333532663&to=1333627917&mode=svg&scale=linear&bgcol=light&gtype=15
		serverusage_api.php?from=1333532663&to=1333627917&mode=svg&scale=log&bgcol=light&gtype=5

### OUTPUT: ###

	The output format can be JSON (JavaScript Object Notation), CSV (tab-separated text values), Base64 encoded serialized array or SVG (Scalable Vector Graphics).

### ADDITIONAL NOTES: ###

	The reference time for all servers should be the standard UTC.
	The latest available time on the ServerUsage-Server aggregated table is always in the past by the value specified by DB_AGGREGATION_DELAY constant (by default 5 minutes).
	The time interval can be calculated as follows:

		polling_interval = 900; // 15 minutes * 60 seconds; must be equal or greater than DB_AGGREGATION_DELAY.
		delay_time = 600; // 10 minutes * 60 seconds; must be equal or greater than (2 * DB_AGGREGATION_DELAY).
		end_time = (current_time - delay_time);
		start_time = (end_time - polling_interval);

HOW-TO CREATE ServerUsage RPMs
------------------------------

This is a short hands-on tutorial on creating RPM files for the ServerUsage project.
For an automatic building script for CentOS and the latest RPM packages please check the CatN Repository: https://github.com/fubralimited/CatN-Repo

NOTE: The sever configuration for ServerUsage-Client and ServerUsage-Server may be different, so this process must be executed in different environments.

## DEVELOPMENT ENVIRONMENT ##

To build RPMs we need a set of development tools.
This is a one-time-only setup, installed by running those commands from a system administration (root) account.
NOTE: You may need to change the the 
	
Install the EPEL repository:

	# rpm -Uvh http://download.fedoraproject.org/pub/epel/6/$(uname -m)/epel-release-6-7.noarch.rpm

Install development tools and Fedora packager:

	# yum install @development-tools
	# yum install fedora-packager

The following packages are required to create ServerUsage RPMs:

	# yum install kernel-devel elfutils-devel sqlite-devel.x86_64 sqlite.x86_64 MariaDB-devel

Install debug packages (change repository if you are not using CentOS):

	wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debug-debuginfo-$(uname -r).rpm
	wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debuginfo-$(uname -r).rpm
	wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debuginfo-common-$(uname -m)-$(uname -r).rpm
	rpm -U --force kernel-debug-debuginfo-$(uname -r).rpm kernel-debuginfo-$(uname -r).rpm kernel-debuginfo-common-$(uname -m)-$(uname -r).rpm

Install SystemTap:

	# yum -y install systemtap systemtap-client systemtap-devel systemtap-runtime systemtap-initscript systemtap-grapher systemtap-sdt-devel systemtap-server systemtap-testsuite

Create a dummy user specifically for creating RPM packages:

	# /usr/sbin/useradd makerpm
	# passwd makerpm

Reboot the machine, log as makerpm user and create the required directory structure in your home directory by executing: 

	$ rpmdev-setuptree

The rpmdev-setuptree program will create the ~/rpmbuild directory and a set of subdirectories (e.g. SPECS and BUILD), which you will use for creating your packages. The ~/.rpmmacros file is also created, which can be used for setting various options. 


## CREATE THE ServerUsage RPMs ##

Download the ServerUsage sources:

	$ cd ~
	$ git clone git://github.com/fubralimited/ServerUsage.git

Copy the SPEC files and source files to rpmbuild dir:
	
	$ cd ~/ServerUsage
	$ export SUVER=$(cat VERSION) 
	
	$ cd ~/ServerUsage/server
	$ cp serverusage_server.spec ~/rpmbuild/SPECS/
	$ tar -zcvf ~/rpmbuild/SOURCES/serverusage_server-$SUVER.tar.gz  *  
	
	$ cd ~/ServerUsage/client
	$ cp serverusage_client.spec ~/rpmbuild/SPECS/
	$ tar -zcvf ~/rpmbuild/SOURCES/serverusage_client-$SUVER.tar.gz  *  
	
	$ cd ~/ServerUsage/client_mdb
	$ cp serverusage_client_mdb.spec ~/rpmbuild/SPECS/
	$ tar -zcvf ~/rpmbuild/SOURCES/serverusage_client_mdb-$SUVER.tar.gz  *    

Create the RPMs:

	$ cd ~/rpmbuild/SPECS/
	$ rpmbuild -ba serverusage_server.spec  
	$ rpmbuild -ba serverusage_client.spec  
	$ rpmbuild -ba serverusage_client_mdb.spec  


The RPMs are now located at ~/rpmbuild/RPMS/$(uname -m)


Install ServerUsage-Server
--------------------------

The ServerUsage-Server RPM must be installed only on the Log Server (the computer receiving the logs from the clients).

As root install the ServerUsage-Server RPM file:

	# rpm -i serverusage_server-6.3.4-1.el6.$(uname -m).rpm
	
Once the RPM is installed you can configure the ServerUsage-Server editing the following file:

	# nano /etc/serverusage_server.conf

The ServerUsage-Server includes a SysV init script to start/stop/restart the service:

	# /etc/init.d/serverusage_server start|stop|status|restart|reload|condrestart

The init script starts the serverusage_tcpreceiver.bin program that listen for incoming TCP connections from the clients, and install a cron job to aggregate the data every 5 minutes.
The raw data received from serverusage_tcpreceiver.bin is stored on a SQLite 3 database (var/lib/serverusage/serverusage.db) table named log_raw. The table containing the aggregated data is called log_agg_hst. The aggregated data is immediately removed from the log_raw table. The data on log_agg_hst older than DB_GARBAGE_TIME seconds is automatically removed.

To start the service at boot you can use the following command:

	# chkconfig serverusage_server on

To extract formatted information from the SQLite database you can use the serverusage_api.php. This file is installed by default in <em>/var/www/serverusage</em> directory, so you have to configure Apache/PHP accordingly or move the script to another position.
The serverusage_api.php allows you to extract filtered information in various formats: JSON (JavaScript Object Notation), CSV (tab-separated text values), Base64 encoded serialized array or SVG (Scalable Vector Graphics). On the same directory where the file serverusage_api.php is, you can find an example HTML file that displays an auto-update graph using the php API.


Install ServerUsage-Client
--------------------------

The ServerUsage-Client RPM must be installed on each client computer to monitor.

As root install the SystemTap-Runtime and ServerUsage-Client RPM files:

	# yum -y install systemtap-runtime
	# rpm -i serverusage_client-6.3.4-1.el6.$(uname -m).rpm

Configure the ServerUsage-Client

	# nano /etc/serverusage_client.conf 

Set the IP address of the Log server where ServerUsage-Server is installed and be sure that the specified TCP port is open on both client and server.

The ServerUsage-Client includes a SysV init script to start/stop/restart the service:

	# /etc/init.d/serverusage_client start|stop|status|restart|reload|condrestart

When the service is started, the serverusage_client.ko SystemTap kernel module is executed via the staprun command and the output is piped to the serverusage_tcpsender.bin to be sent to the Log server via a TCP connection. If the connection is broken or the Log server is not responding, the log files are temporarily stored on <em>/var/log/serverusage_cache.log</em> file and resent as soon the TCP connection is restored.

To start the service at boot you can use the following command:

	# chkconfig serverusage_client on


Install ServerUsage-Client-MDB
------------------------------

The ServerUsage-Client RPM must be installed on the computer containing the MariaDB database.

As root install the ServerUsage-Client RPM file:

	# rpm -i serverusage_client_mdb-6.3.4-1.el6.$(uname -m).rpm

Configure the ServerUsage-Client

	# nano /etc/serverusage_client_mdb.conf 

Set the IP address of the Log server where ServerUsage-Server is installed and be sure that the specified TCP port is open on both client and server.

The ServerUsage-Client includes a SysV init script to start/stop/restart the service:

	# /etc/init.d/serverusage_client_mdb start|stop|status|restart|reload|condrestart

To start the service at boot you can use the following command:

	# chkconfig serverusage_client_mdb on
