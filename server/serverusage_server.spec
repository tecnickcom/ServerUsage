%define release 1

Name:           serverusage_server
Version:        4.8.0
Release:        %{release}%{?dist}
Summary:        ServerUsage-Server collects logs data via TCP from ServerUsage-Client

Group:          Applications/System
License:        AGPLv3
URL:            https://github.com/fubralimited/ServerUsage
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  sqlite-devel > 3.6.0, elfutils-devel
Requires:       httpd >= 2.0.0, sqlite >= 3.6.0, php >= 5.3.0, php-common >= 5.3.0, php-pdo >= 5.3.0

%description
The ServerUsage-Server is a program to collect and process log data sent by
ServerUsage-Client instances on remote computers. This program listen on a TCP
port for incoming log data and store them on a SQLite table. An shell script
(serverusage_dbagg.sh) is executed periodically by a cron job to aggregate data
on another SQLite table.

%prep
%setup -q -c %{name}-%{version}

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT
make clean

%files
%defattr(-,root,root,-)
%doc README LICENSE
%{_bindir}/serverusage_tcpreceiver.bin
%{_bindir}/serverusage_dbagg.sh
%{_sysconfdir}/serverusage_server.conf
%{_initrddir}/serverusage_server
%{_mandir}/man8/serverusage_server.8.gz
%{_datadir}/serverusage/serverusage_database.sql
%{_sharedstatedir}/serverusage/serverusage.db
/var/www/serverusage/serverusage_api.php
/var/www/serverusage/serverusage_svg.html

%changelog
* Thu May 15 2012 Nicola Asuni
- SQLite is now used as back-end database.

* Thu Apr 26 2012 Nicola Asuni
- PHP util script was added and requirements were updated.

* Wed Apr 25 2012 Nicola Asuni
- First version for RPM packaging.
