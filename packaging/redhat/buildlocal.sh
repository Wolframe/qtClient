#!/bin/sh

# Note: Create a ~/.rpmmacros to set number of CPUs for
# parallel building like this:
# %_smp_mflags -j24

# Of course you need build toold and rpm-build, also a ~/rpmbuild directory
# set up

# the Centos version is set below as 'centos_version 630' (OSB service
# linguo for Centos 6.3)

VERSION=0.0.4
RPMBUILD=$HOME/rpmbuild
#OSB_PLATFORM=

rm -rf $RPMBUILD/BUILDROOT $RPMBUILD/BUILD $RPMBUILD/RPMS/*/* \
	$RPMBUILD/SRPMS/* $RPMBUILD/SPECS/log
touch $RPMBUILD/SPECS/log

rm -f $RPMBUILD/SOURCES/wolfclient_$VERSION.tar.gz

# This is actually wrong for openSUSE, but we don't build it locally, only
# on OBS:
qmake-qt4 wolfclient.pro -config release -recursive
make dist-gz

cp wolfclient-$VERSION.tar.gz $RPMBUILD/SOURCES/wolfclient_$VERSION.tar.gz
cp packaging/redhat/wolfclient.spec $RPMBUILD/SPECS/wolfclient.spec

cd $RPMBUILD/SPECS

if test -f /opt/intel/bin/iccvars.sh; then
	if test -f /opt/intel/bin/iccvars.sh; then
		MACHINE_ARCH=`uname -m`
		if test "$MACHINE_ARCH" = "x86_64"; then
			ICC_ARCH="intel64"
			export QMAKESPEC="linux-icc-64"
		else
			if test "$MACHINE_ARCH" = "i686"; then
				ICC_ARCH="ia32"
				export QMAKESPEC="linux-icc-32"
			else
				print "ERROR: Unknown Intel architecture $MACHINE_ARCH!"
				global_unlock
				exit 1
			fi
		fi
		. /opt/intel/bin/iccvars.sh $ICC_ARCH
	fi
	export QMAKE_CXX='ccache icpc'
	
	# Note: additionally hack /usr/lib/qt4/mkspecs/linux-icc/qmake.conf
	# and set QMAKE_CXX to 'ccache icpc'
else
	export QMAKE_CXX='ccache g++'
fi

rpmbuild -ba --define "$OSB_PLATFORM" wolfclient.spec

echo "Build done."
