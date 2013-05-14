#!/bin/sh

VERSION=0.0.1
OSC_HOME=$HOME/home:andreas_baumann/qtClient

rm -f wolframe-qtclient-$VERSION.tar.gz
make distclean
tar zcvf wolframe-qtclient-$VERSION.tar.gz *
cp wolframe-qtclient-$VERSION.tar.gz $OSC_HOME/wolframe-qtclient_$VERSION.tar.gz
cp redhat/qtClient.spec $OSC_HOME/qtClient.spec

SIZE=`stat -c '%s' $OSC_HOME/wolframe-qtclient_$VERSION.tar.gz`
CHKSUM=`md5sum $OSC_HOME/wolframe-qtclient_$VERSION.tar.gz | cut -f 1 -d' '`

cat contrib/osc/qtClient.dsc > $OSC_HOME/qtClient.dsc
echo " $CHKSUM $SIZE wolframe-qtclient$VERSION.tar.gz" >> $OSC_HOME/qtClient.dsc

cat contrib/osc/PKGBUILD > $OSC_HOME/PKGBUILD
echo "md5sums=('$CHKSUM' '$CHKSUM2' '$CHKSUM3')" >> $OSC_HOME/PKGBUILD
