ServerUsage - README
====================

+ Name: ServerUsage

+ Version: 4.5.0

+ Release date: 2012-05-18

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

The ServerUsage system is designed to collect and process statistic information from computers
running a GNU-Linux Operating System.

This project is composed by 2 sub-projects:


## ServerUsage-Client ##

The ServerUsage-Client program is composed by two main sections: the SystemTap
(http://sourceware.org/systemtap) serverusage_client.ko kernel module to collect
and output usage statistics of the machine where it is installed, and the
serverusage_tcpsender.bin to send the output to a log server via TCP.


## ServerUsage-Server ##

The ServerUsage-Server program listen on a TCP port for incoming log data 
(from ServerUsage-Client) and store them on a SQLite table. An SQL file 
(serverusage_dbagg.sql) is executed periodically to aggregate data on another 
table and delete obsolete data.

The serverusage_api.php script can be remotely used to extract formatted data 
from the database or display graphs.


## GENERAL USAGE SCHEMA ##
	
	(ServerUsage-Client) - - - - - - - - - - - -> (ServerUsage-Server)
	
	([serverusage.ko]==>[tcpsender.bin]) - - - -> ([tcpreceiver]==>[RAW DATABASE TABLE])
	
	[RAW DATABASE TABLE]==>[serverusageagg.sql]==>[AGGREGATE DATABASE TABLE]
	
	[AGGREGATE DATABASE TABLE]==>[srvusgapi.php]==>[Data with selected format or SVG graph]


HOW-TO CREATE ServerUsage RPMs
------------------------------

This is a short hands-on tutorial on creating RPM files for the ServerUsage project.
NOTE: The sever configuration for ServerUsage-Client and ServerUsage-Server may be different, so this process must be executed in different environments.


## DEVELOPMENT ENVIRONMENT ##

To build RPMs we need a set of development tools.
This is a one-time-only setup, installed by running those commands from a system administration (root) account.
NOTE: You may need to change the the 
	
Install the EPEL repository:

	# rpm -Uvh http://download.fedoraproject.org/pub/epel/6/$(uname -m)/epel-release-6-6.noarch.rpm

Install development tools and Fedora packager:

	# yum install @development-tools
	# yum install fedora-packager

The following packages are required to create ServerUsage RPMs:

	# yum install kernel-devel elfutils-devel sqlite-devel.x86_64 sqlite.x86_64

For CentOS 6:

	# wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debug-debuginfo-$(uname -r).rpm
	# wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debuginfo-$(uname -r).rpm
	# wget http://debuginfo.centos.org/6/$(uname -m)/kernel-debuginfo-common-$(uname -m)-$(uname -r).rpm

For Scientific Linux 6:

	# wget http://ftp1.scientificlinux.org/linux/scientific/6rolling/archive/debuginfo/kernel-debug-debuginfo-$(uname -r).rpm
	# wget http://ftp1.scientificlinux.org/linux/scientific/6rolling/archive/debuginfo/kernel-debuginfo-$(uname -r).rpm
	# wget http://ftp1.scientificlinux.org/linux/scientific/6rolling/archive/debuginfo/kernel-debuginfo-common-$(uname -m)-$(uname -r).rpm

Install packages:

	# yum install kernel-debug-debuginfo-$(uname -r).rpm kernel-debuginfo-$(uname -r).rpm kernel-debuginfo-common-$(uname -m)-$(uname -r).rpm

The following packages are required to create systemtap RPMs:

	# yum install sqlite-devel crash-devel rpm-devel nss-devel avahi-devel latex2html xmlto xmlto-tex publican publican-fedora gtkmm24-devel libglademm24-devel boost-devel

Create a dummy user specifically for creating RPM packages:

	# /usr/sbin/useradd makerpm
	# passwd makerpm

Reboot the machine, log as makerpm user and create the required directory structure in your home directory by executing: 

	$ rpmdev-setuptree

The rpmdev-setuptree program will create the ~/rpmbuild directory and a set of subdirectories (e.g. SPECS and BUILD), which you will use for creating your packages. The ~/.rpmmacros file is also created, which can be used for setting various options. 


## CREATE THE SYSTEMTAP RPM ##

copy the systemtap.spec file on ~/rpmbuild/SPECS

	$ cp systemtap.spec ~/rpmbuild/SPECS

copy the systemtap 1.7 source file into ~/rpmbuild/SOURCES

	$ cd ~/rpmbuild/SOURCES
	$ wget http://sourceware.org/systemtap/ftp/releases/systemtap-1.7.tar.gz

create the SystemTap RPMs

	$ cd ~/rpmbuild/SPECS/
	$ rpmbuild -ba systemtap.spec


## INSTALL THE THE SYSTEMTAP RPM ##

	$ cd ~/rpmbuild/RPMS/$(uname -m)

As root, remove previous SystemTap installations:

	# yum remove systemtap\*

Reboot the machine and as root install new SystemTap RPMs:

	# rpm -i systemtap-1.7-1.el6.$(uname -m).rpm systemtap-client-1.7-1.el6.$(uname -m).rpm systemtap-debuginfo-1.7-1.el6.$(uname -m).rpm systemtap-devel-1.7-1.el6.$(uname -m).rpm systemtap-grapher-1.7-1.el6.$(uname -m).rpm systemtap-initscript-1.7-1.el6.$(uname -m).rpm systemtap-runtime-1.7-1.el6.$(uname -m).rpm systemtap-sdt-devel-1.7-1.el6.$(uname -m).rpm systemtap-server-1.7-1.el6.$(uname -m).rpm


## CREATE THE ServerUsage RPMs ##

Download the ServerUsage sources:

	$ cd ~
	$ git clone git://github.com/fubralimited/ServerUsage.git

Copy the SPEC files and source files to rpmbuild dir (replace the version number with the correct value):
	
	$ cd ~/ServerUsage
	$ export SUVER=$(cat VERSION) 
	
	$ cd ~/ServerUsage/client
	$ cp serverusage_client.spec ~/rpmbuild/SPECS/
	$ tar -zcvf ~/rpmbuild/SOURCES/serverusage_client-$SUVER.tar.gz  *
	
	$ cd ~/ServerUsage/server
	$ cp serverusage_server.spec ~/rpmbuild/SPECS/
	$ tar -zcvf ~/rpmbuild/SOURCES/serverusage_server-$SUVER.tar.gz  *

Create the RPMs:

	$ cd ~/rpmbuild/SPECS/
	$ rpmbuild -ba serverusage_client.spec
	$ rpmbuild -ba serverusage_server.spec


The RPMs are now located at ~/rpmbuild/RPMS/$(uname -m)


INSTALL SERVERUSAGE SERVER:
---------------------------

As root install the ServerUsage-Server RPM file:

	# rpm -i serverusage_server-4.5.0-1.el6.$(uname -m).rpm 
	
Configure the ServerUsage-Server

	# nano /etc/serverusage_server.conf

Start the service

	# /etc/init.d/serverusage_server start

Start the service at boot:

	# chkconfig serverusage_server on

INSTALL SERVERUSAGE CLIENT:
---------------------------

As root install the SystemTap runtime and ServerUsage-Client RPM files:

	# rpm -i systemtap-runtime-1.7-1.el6.$(uname -m).rpm 
	# rpm -i serverusage_client-4.5.0-1.el6.$(uname -m).rpm

Configure the ServerUsage-Client

	# nano /etc/serverusage_client.conf 

Start the service

	# /etc/init.d/serverusage_client start

Start the service at boot:

	# chkconfig serverusage_client on

