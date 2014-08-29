# Wolframe wolfclient spec file
#
# Copyright (C) 2011 - 2014 Project Wolframe

# Set distribution based on some OpenSuse and distribution macros
# this is only relevant when building on https://build.opensuse.org
###

%define rhel 0
%define rhel5 0
%define rhel6 0
%define rhel7 0
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
%if 0%{?rhel_version} >= 700 && 0%{?rhel_version} <= 799
%define dist rhel7
%define rhel 1
%define rhel7 1
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

%define centos7 0
%if 0%{?centos_version} >= 700 && 0%{?centos_version} <= 799
%define dist centos7
%define centos 1
%define centos7 1
%endif

%define scilin 0
%define scilin5 0
%if 0%{?scilin_version} >= 500 && 0%{?scilin_version} <= 599
%define dist scilin5
%define scilin 1
%define scilin5 1
%endif

%define scilin6 0
%if 0%{?scilin_version} >= 600 && 0%{?scilin_version} <= 699
%define dist scilin6
%define scilin 1
%define scilin6 1
%endif

%define scilin7 0
%if 0%{?scilin_version} >= 700 && 0%{?scilin_version} <= 799
%define dist scilin7
%define scilin 1
%define scilin7 1
%endif

%define fedora 0
%define fc19 0
%if 0%{?fedora_version} == 19
%define dist fc19
%define fc19 1
%define fedora 1
%endif
%define fc20 0
%if 0%{?fedora_version} == 20
%define dist fc20
%define fc20 1
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
Name: wolfclient
Version: 0.0.5
Release: 0.1
License: Wolframe License
Group: Application/Business
Source: %{name}_%{version}.tar.gz

URL: http://www.wolframe.net/

BuildRoot: %{_tmppath}/%{name}-root

%if %{rhel} || %{centos} || %{scilin} || %{fedora}
BuildRequires: qt4-devel >= 4.5
Requires: qt4 >= 4.5
# somehow there is no c++ compiler per default installed on RHEL6/7
%if %{rhel6} || %{centos6} || %{scilin6} || %{rhel7} || %{centos7} || %{scilin7}
BuildRequires: gcc-c++
%endif
%endif
%if %{suse} || %{sles}
BuildRequires: libqt4-devel >= 4.5
Requires: libqt4 >= 4.5
%endif

# installation directory for designer plugins
%define plugindir %_libdir/qt4/plugins

%description
Client for the Wolframe system.

%prep
%setup -q

%build
%if %{rhel} || %{centos} || %{scilin} || %{fedora}
qmake-qt4 wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%else
qmake wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%endif
make %{?_smp_mflags} LIBDIR=%{_libdir}/wolframe

%install
rm -rf $RPM_BUILD_ROOT

make INSTALL_ROOT=$RPM_BUILD_ROOT LIBDIR=%{_libdir} install

%clean
%{__rm} -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_ROOT

%files
%defattr( -, root, root )
%{_bindir}/wolfclient
%dir %{_libdir}/wolframe
%{_libdir}/wolframe/libskeleton.so.0.0.0
%{_libdir}/wolframe/libskeleton.so.0
%{_libdir}/wolframe/libqtwolfclient.so.0.0.0
%{_libdir}/wolframe/libqtwolfclient.so.0
%plugindir/designer/libfilechooser.so
%plugindir/designer/libpicturechooser.so
# those two are temporary till the plugin system in wolfclient kicks in :-)
%{_libdir}/wolframe/libfilechooser.so
%{_libdir}/wolframe/libpicturechooser.so

%package devel
Summary: Wolframe Qt client development files
Group: Application/Business

%description devel
The libraries and header files used for development with Wolframe.

Requires: %{name} >= %{version}-%{release}

%files devel
%defattr( -, root, root )
%{_libdir}/wolframe/libskeleton.so.0.0
%{_libdir}/wolframe/libskeleton.so  
%{_libdir}/wolframe/libqtwolfclient.so.0.0
%{_libdir}/wolframe/libqtwolfclient.so    

%changelog
* Fri Aug 29 2014 Andreas Baumann <abaumann@yahoo.com> 0.0.5-0.1
- intermediary release

* Tue Jun 20 2014 Andreas Baumann <abaumann@yahoo.com> 0.0.4-0.1
- intermediary release

* Tue Dec 04 2013 Andreas Baumann <abaumann@yahoo.com> 0.0.3-0.1
- intermediary release

* Tue Dec 04 2013 Andreas Baumann <abaumann@yahoo.com> 0.0.2-0.1
- intermediary release

* Tue May 14 2013 Andreas Baumann <abaumann@yahoo.com> 0.0.1-0.1
- preliminary release
