#!/bin/sh

# Note: put dot_home_rpmmacros to ~/.rpmmacros to set number of CPUs for
# parallel building

# Of course you need build toold and rpm-build, also a ~/rpmbuild directory
# set up

# the Centos version is set below as 'centos_version 630' (OSB service
# linguo for Centos 6.3)

VERSION=0.0.1
RPMBUILD=$HOME/rpmbuild
#OSB_PLATFORM=

rm -rf $RPMBUILD/BUILDROOT $RPMBUILD/BUILD $RPMBUILD/RPMS/*/* \
	$RPMBUILD/SRPMS/* $RPMBUILD/SPECS/log
touch $RPMBUILD/SPECS/log

rm -f wolframe_client-$VERSION.tar.gz
rm -f $RPMBUILD/SOURCES/wolframe_client_$VERSION.tar.gz

make \
	WITH_SSL=1 WITH_EXPECT=1 WITH_QT=1 WITH_PAM=1 WITH_SASL=1 \
	WITH_SQLITE3=1 WITH_PGSQL=1 WITH_LUA=1 WITH_LIBXML2=1 WITH_LIBXSLT=1 \
	WITH_ICU=1 dist-gz >/dev/null 2>&1

cp wolframe_client-$VERSION.tar.gz $RPMBUILD/SOURCES/wolframe_client_$VERSION.tar.gz
cp redhat/qtClient.spec $RPMBUILD/SPECS/qtClient.spec

cd $RPMBUILD/SPECS

echo "Building started, check with 'tail -f $RPMBUILD/SPECS/log'."

export CC='ccache gcc'
export CXX='ccache g++'
rpmbuild -ba --define "$OSB_PLATFORM" qtClient.spec > log 2>&1

echo "Build done."
