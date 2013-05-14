# Wolframe qtClient spec file
#
# Copyright (C) 2011-2013 Project Wolframe

# Set distribution based on some OpenSuse and distribution macros
# this is only relevant when building on https://build.opensuse.org
###

%define rhel 0
%define rhel5 0
%define rhel6 0
%if 0%{?rhel_version} >= 500 && 0%{?rhel_version} <= 599
%define dist rhel5
%define rhel 1
%define rhel5 1
%endif
%if 0%{?rhel_version} >= 600 && 0%{?rhel_version} <= 699
%define dist rhel6
%define rhel 1
%define rhel6 1
%endif

%define centos 0
%define centos5 0
%if 0%{?centos_version} >= 500 && 0%{?centos_version} <= 599
%define dist centos5
%define centos 1
%define centos5 1
%endif

%define centos6 0
%if 0%{?centos_version} >= 600 && 0%{?centos_version} <= 699
%define dist centos6
%define centos 1
%define centos6 1
%endif

%define fedora 0
%define fc17 0
%if 0%{?fedora_version} == 17
%define dist fc17
%define fc17 1
%define fedora 1
%endif
%define fc18 0
%if 0%{?fedora_version} == 18
%define dist fc18
%define fc18 1
%define fedora 1
%endif

%define suse 0
%define osu114 0
%define osu121 0
%define osu122 0
%define osu131 0
%if 0%{?suse_version} == 1140
%define dist osu114
%define osu114 1
%define suse 1
%endif
%if 0%{?suse_version} == 1210
%define dist osu121
%define osu121 1
%define suse 1
%endif
%if 0%{?suse_version} >= 1220
%define dist osu122
%define osu122 1
%define suse 1
%endif
%if 0%{?suse_version} >= 1310
%define dist osu131
%define osu122 1
%define suse 1
%endif

%define sles 0
%if 0%{?sles_version} == 11
%define dist sle11
%define sles 1
%endif

Summary: Qt client for Wolframe
Name: wolframe-qtclient
Version: 0.0.1
Release: 0.1
License: Wolframe License
Group: Application/Business
Source: %{name}_%{version}.tar.gz

%if %{rhel} || %{centos} || %{fedora}
BuildRequires: qt4-devel >= 4.5
Requires: qt4 >= 4.5
%endif
%if %{suse} || %{sles}
BuildRequires: libqt4-devel >= 4.5
Requires: libqt4 >= 4.5
%endif

%description
Generic Qt client for the Wolframe system.

%prep
%setup -q

%build
%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 qtClient.pro PREFIX=%{_prefix}
%else
qmake qtClient.pro PREFIX=%{_prefix}
%endif
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make INSTALL_ROOT=$RPM_BUILD_ROOT install

%clean
%{__rm} -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_ROOT

%if %{with_qt4}
%files qtclient
%defattr( -, root, root )
# funny, why?!
%if !%{sles}
%dir %{_bindir}
%endif
%{_bindir}/qtclient
%dir %{_libdir}/wolframe/plugins/
%{_libdir}/wolframe/plugins/libfilechooser.so
%{_libdir}/wolframe/plugins/libpicturechooser.so
%{_libdir}/wolframe/plugins/libwimagelistwidget.so
%endif


%changelog
* Tue May 14 2013 Andreas Baumann <abaumann@yahoo.com> 0.0.1-0.1
- preliminary release
