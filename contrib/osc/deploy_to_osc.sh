#!/bin/sh

VERSION=0.0.1
OSC_HOME=$HOME/home:andreas_baumann/wolfclient

rm -f wolfclient-$VERSION.tar.gz
rm -f $RPMBUILD/SOURCES/wolfclient_$VERSION.tar.gz

make distclean
mkdir /tmp/wolfclient-$VERSION
cp -av * /tmp/wolfclient-$VERSION
cd /tmp
tar zcvf wolfclient-$VERSION.tar.gz wolfclient-$VERSION
cd -
mv /tmp/wolfclient-$VERSION.tar.gz .
rm -rf /tmp/wolfclient-$VERSION

cp wolfclient-$VERSION.tar.gz $OSC_HOME/wolfclient_$VERSION.tar.gz
cp redhat/wolfclient.spec $OSC_HOME/wolfclient.spec

SIZE=`stat -c '%s' $OSC_HOME/wolfclient_$VERSION.tar.gz`
CHKSUM=`md5sum $OSC_HOME/wolfclient_$VERSION.tar.gz | cut -f 1 -d' '`


cat contrib/osc/wolfclient.dsc > $OSC_HOME/wolfclient.dsc
echo " $CHKSUM $SIZE wolfclient_$VERSION.tar.gz" >> $OSC_HOME/wolfclient.dsc

cat contrib/osc/PKGBUILD > $OSC_HOME/PKGBUILD
echo "md5sums=('$CHKSUM' '$CHKSUM2' '$CHKSUM3')" >> $OSC_HOME/PKGBUILD

# the revision of the git branch we are currently building (master hash at the moment)
git rev-parse HEAD > $OSC_HOME/GIT_VERSION
