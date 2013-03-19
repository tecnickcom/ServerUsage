%define release 1

Name:           serverusage_client_mdb
Version:        6.3.7
Release:        %{release}%{?dist}
Summary:        ServerUsage-Client-MDB collects MariaDB usage statistics and send them to a remote log server via TCP

Group:          Applications/System
License:        GPLv2
URL:            https://github.com/fubralimited/ServerUsage
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  elfutils-devel, MariaDB-devel
Requires:       MariaDB-server

Requires(preun): chkconfig
Requires(preun): initscripts
Requires(postun): initscripts

%description
ServerUsage-Client-MDB is a tool to collect MariaDB usage statistics and send 
them to a remote log server via TCP. This tool is composed by two main parts:
the serverusage_client_mdb.bin to collect usage statistics from the database and
the serverusage_tcpsender_mdb.bin to send the output to a log server via TCP.

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
%{_bindir}/serverusage_client_mdb.bin
%{_bindir}/serverusage_tcpsender_mdb.bin
%config(noreplace) %{_sysconfdir}/serverusage_client_mdb.conf
%{_initrddir}/serverusage_client_mdb
%{_mandir}/man8/serverusage_client_mdb.8.gz

%preun
if [ $1 -eq 0 ] ; then
	# uninstall: stop service
	/sbin/service serverusage_client_mdb stop >/dev/null 2>&1
	/sbin/chkconfig --del serverusage_client_mdb
fi

%postun
if [ $1 -eq 1 ] ; then
	# upgrade: restart service if was running
	/sbin/service serverusage_client_mdb condrestart >/dev/null 2>&1 || :
fi

%changelog
* Thu Aug 21 2012 Nicola Asuni
- Added %config(noreplace)
- Added preun and postun sections

* Wed Aug 15 2012 Nicola Asuni
- First version for RPM packaging.
