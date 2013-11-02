#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/slackbuild
ARCH=`uname -m`
if test "x$ARCH" = "xx86_64"; then
	LIBDIR="/usr/lib64"
else
if test "x$ARCH" = "xi686"; then
	LIBDIR="/usr/lib"
else
	echo "ERROR: Unknown slackware architecture '$ARCH'"
	exit 1
fi
fi
rm -rf $PKGBUILD/BUILD $PKGBUILD/PKG

mkdir -p $PKGBUILD $PKGBUILD/BUILD $PKGBUILD/PKG $PKGBUILD/PKGS/$ARCH

rm -f wolfclient-$VERSION.tar.gz
rm -f $RPMBUILD/BUILD/wolfclient_$VERSION.tar.gz

make distclean
mkdir /tmp/wolfclient-$VERSION
cp -a * /tmp/wolfclient-$VERSION
cd /tmp
tar zcf wolfclient-$VERSION.tar.gz wolfclient-$VERSION
cd -
mv /tmp/wolfclient-$VERSION.tar.gz .
rm -rf /tmp/wolfclient-$VERSION

cp wolfclient-$VERSION.tar.gz $PKGBUILD/BUILD/.
cd $PKGBUILD/BUILD
tar zxf wolfclient-$VERSION.tar.gz
cd wolfclient-$VERSION

qmake -config release -recursive wolfclient.pro PREFIX=/usr LIBDIR=$LIBDIR
make CXX='ccache g++' PREFIX=/usr LIBDIR=$LIBDIR
# needs X11 or headless X11 server, disabled for now
#make test

find . -name Makefile -exec rm -f {} \;
qmake -config release -recursive wolfclient.pro PREFIX=/usr LIBDIR=$LIBDIR
make INSTALL_ROOT=$PKGBUILD/PKG install PREFIX=/usr LIBDIR=$LIBDIR

mkdir $PKGBUILD/PKG/install
cp packaging/slackware/slack-desc $PKGBUILD/PKG/install/.
cp packaging/slackware/doinst-$ARCH.sh $PKGBUILD/PKG/install/doinst.sh
cd $PKGBUILD/PKG
makepkg -l y -c n $PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION.tgz

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
