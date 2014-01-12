#!/bin/sh

# Note: Create a ~/.rpmmacros to set number of CPUs for
# parallel building like this:
# %_smp_mflags -j24

# Of course you need build toold and rpm-build, also a ~/rpmbuild directory
# set up

# the Centos version is set below as 'centos_version 630' (OSB service
# linguo for Centos 6.3)

VERSION=0.0.3
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

export CC='ccache gcc'
export CXX='ccache g++'
rpmbuild -ba --define "$OSB_PLATFORM" wolfclient.spec

echo "Build done."
