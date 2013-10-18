#!/bin/sh

VERSION=0.0.1
OSC_HOME=$HOME/home:andreas_baumann/wolfclient

rm -f wolfclient-$VERSION.tar.gz
rm -f $RPMBUILD/SOURCES/wolfclient_$VERSION.tar.gz

test ! -f Makefile || make distclean
mkdir /tmp/wolfclient-$VERSION
cp -a * /tmp/wolfclient-$VERSION
cd /tmp
tar zcf wolfclient-$VERSION.tar.gz wolfclient-$VERSION
cd -
mv /tmp/wolfclient-$VERSION.tar.gz .
rm -rf /tmp/wolfclient-$VERSION

cp wolfclient-$VERSION.tar.gz $OSC_HOME/wolfclient_$VERSION.tar.gz
cp packaging/redhat/wolfclient.spec $OSC_HOME/wolfclient.spec

SIZE=`stat -c '%s' $OSC_HOME/wolfclient_$VERSION.tar.gz`
CHKSUM=`md5sum $OSC_HOME/wolfclient_$VERSION.tar.gz | cut -f 1 -d' '`


cat packaging/obs/wolfclient.dsc > $OSC_HOME/wolfclient.dsc
echo " $CHKSUM $SIZE wolfclient_$VERSION.tar.gz" >> $OSC_HOME/wolfclient.dsc

cat packaging/obs/PKGBUILD > $OSC_HOME/PKGBUILD
echo "md5sums=('$CHKSUM' '$CHKSUM2' '$CHKSUM3')" >> $OSC_HOME/PKGBUILD

# the revision of the git branch we are currently building (master hash at the moment)
git rev-parse HEAD > $OSC_HOME/GIT_VERSION
