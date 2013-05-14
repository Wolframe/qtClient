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

rm -f wolframe-qtclient-$VERSION.tar.gz
rm -f $RPMBUILD/SOURCES/wolframe-qtclient_$VERSION.tar.gz

make distclean
mkdir /tmp/wolframe-qtclient-$VERSION
cp -av * /tmp/wolframe-qtclient-$VERSION
cd /tmp
tar zcvf wolframe-qtclient-$VERSION.tar.gz wolframe-qtclient-$VERSION
cd -
mv /tmp/wolframe-qtclient-$VERSION.tar.gz .
rm -rf /tmp/wolframe-qtclient-$VERSION

cp wolframe-qtclient-$VERSION.tar.gz $RPMBUILD/SOURCES/wolframe-qtclient_$VERSION.tar.gz
cp redhat/qtClient.spec $RPMBUILD/SPECS/qtClient.spec

cd $RPMBUILD/SPECS

echo "Building started, check with 'tail -f $RPMBUILD/SPECS/log'."

export CC='ccache gcc'
export CXX='ccache g++'
rpmbuild -ba --define "$OSB_PLATFORM" qtClient.spec > log 2>&1

echo "Build done."
