%define release 1

Name:           serverusage_client
Version:        4.5.0
Release:        %{release}%{?dist}
Summary:        ServerUsage-Client collects server usage statistics and send them to a remote log server via TCP

Group:          Applications/System
License:        AGPLv3
URL:            https://github.com/fubralimited/ServerUsage
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  kernel-devel, elfutils-devel, kernel-debug-debuginfo, kernel-debuginfo, kernel-debuginfo-common-x86_64
Requires:       systemtap-runtime >= 1.7

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
%{_sysconfdir}/serverusage_client.conf
%{_initrddir}/serverusage_client
%{_mandir}/man8/serverusage_client.8.gz

%changelog
* Thu May 15 2012 Nicola Asuni
- Rebuild to sync with the new server version.

* Thu Apr 26 2012 Nicola Asuni
- Required packages were updated to include the correct version.

* Wed Apr 25 2012 Nicola Asuni
- First version for RPM packaging.
