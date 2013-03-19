%define release 1

Name:           serverusage_client
Version:        6.3.7
Release:        %{release}%{?dist}
Summary:        ServerUsage-Client collects server usage statistics and send them to a remote log server via TCP

Group:          Applications/System
License:        AGPLv3
URL:            https://github.com/fubralimited/ServerUsage
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  kernel-devel, elfutils-devel, kernel-debug-debuginfo, kernel-debuginfo, kernel-debuginfo-common-x86_64
Requires:       systemtap-runtime >= 1.7

Requires(preun): chkconfig
Requires(preun): initscripts
Requires(postun): initscripts

%description
ServerUsage-Client is a tool to collect system statistics and send them to a
remote log server via TCP. This tool is composed by two main parts: a SystemTap
instrumentation utility to collect data on the operation of the system and an
utility to send the collected data to a remote log server via TCP.

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
%{_bindir}/serverusage_client.ko
%{_bindir}/serverusage_tcpsender.bin
%config(noreplace) %{_sysconfdir}/serverusage_client.conf
%{_initrddir}/serverusage_client
%{_mandir}/man8/serverusage_client.8.gz

%preun
if [ $1 -eq 0 ] ; then
	# uninstall: stop service
	/sbin/service serverusage_client stop >/dev/null 2>&1
	/sbin/chkconfig --del serverusage_client
fi

%postun
if [ $1 -eq 1 ] ; then
	# upgrade: restart service if was running
	/sbin/service serverusage_client condrestart >/dev/null 2>&1 || :
fi

%changelog
* Thu Aug 21 2012 Nicola Asuni
- Added %config(noreplace)
- Added preun and postun sections

* Thu May 15 2012 Nicola Asuni
- Rebuild to sync with the new server version.

* Thu Apr 26 2012 Nicola Asuni
- Required packages were updated to include the correct version.

* Wed Apr 25 2012 Nicola Asuni
- First version for RPM packaging.
