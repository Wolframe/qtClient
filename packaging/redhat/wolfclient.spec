# Wolframe wolfclient spec file
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
%define fc18 0
%if 0%{?fedora_version} == 18
%define dist fc18
%define fc18 1
%define fedora 1
%endif
%define fc19 0
%if 0%{?fedora_version} == 19
%define dist fc19
%define fc19 1
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
Version: 0.0.1
Release: 0.1
License: Wolframe License
Group: Application/Business
Source: %{name}_%{version}.tar.gz

URL: http://www.wolframe.net/

BuildRoot: %{_tmppath}/%{name}-root

%if %{rhel} || %{centos} || %{fedora}
BuildRequires: qt4-devel >= 4.5
Requires: qt4 >= 4.5
# somehow there is no c++ compiler per default installed on RHEL6/Centos6
%if %{rhel6} || %{centos6}
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
%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%else
qmake wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%endif
make %{?_smp_mflags} LIBDIR=%{_libdir}/wolframe

%install
rm -rf $RPM_BUILD_ROOT

# regenarete Makefiles here, as files have to exist in order for
# the install rules to be generated
%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%else
qmake wolfclient.pro -config release -recursive PREFIX=%{_prefix} LIBDIR=%{_libdir}/wolframe
%endif

make INSTALL_ROOT=$RPM_BUILD_ROOT LIBDIR=%{_libdir} install

%clean
%{__rm} -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_ROOT

%files
%defattr( -, root, root )
# funny, why?!
#%if !%{sles} && !%{fc18}
#%dir %{_bindir}
#%endif
%{_bindir}/wolfclient
%dir %{_libdir}/wolframe
%{_libdir}/wolframe/libskeleton.so.0.0.1
%{_libdir}/wolframe/libskeleton.so.0
%{_libdir}/wolframe/libqtwolfclient.so.0.0.1
%{_libdir}/wolframe/libqtwolfclient.so.0
%plugindir/designer/libfilechooser.so
%plugindir/designer/libpicturechooser.so
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
* Tue May 14 2013 Andreas Baumann <abaumann@yahoo.com> 0.0.1-0.1
- preliminary release
