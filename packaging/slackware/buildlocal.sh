#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/slackbuild
ARCH=`uname -m`
rm -rf $PKGBUILD/BUILD $PKGBUILD/PKG

mkdir $PKGBUILD/BUILD $PKGBUILD/PKG $PKGBUILD/SOURCES $PKGBUILD/PKGS/$ARCH

rm -f wolfclient-$VERSION.tar.gz
rm -f $RPMBUILD/SOURCES/wolfclient_$VERSION.tar.gz

make distclean
mkdir /tmp/wolfclient-$VERSION
cp -a * /tmp/wolfclient-$VERSION
cd /tmp
tar zcf wolfclient-$VERSION.tar.gz wolfclient-$VERSION
cd -
mv /tmp/wolfclient-$VERSION.tar.gz .
rm -rf /tmp/wolfclient-$VERSION

cp wolfclient-$VERSION.tar.gz $PKGBUILD/SOURCES/.
cd $PKGBUILD/SOURCES
tar zxf wolfclient-$VERSION.tar.gz
cd wolfclient-$VERSION

export CXX='ccache g++'
qmake -config release -recursive wolfclient.pro
make
# needs X11 or headless X11 server, disabled for now
#make test

qmake -config release -recursive wolfclient.pro
make INSTALL_ROOT=$PKGBUILD/PKG install

mkdir $PKGBUILD/PKG/install
cp packaging/slackware/slack-desc $PKGBUILD/PKG/install/.
cd $PKGBUILD/PKG
makepkg -c n $PKGBUILD/PKGS/$ARCH/.

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
