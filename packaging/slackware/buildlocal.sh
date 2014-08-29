#!/bin/sh

VERSION=0.0.5
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

rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/* \
	$PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION*.tgz

mkdir -p $PKGBUILD $PKGBUILD/BUILD $PKGBUILD/PKG $PKGBUILD/PKGS/$ARCH

rm -f $PKGBUILD/BUILD/wolfclient_$VERSION.tar.gz

qmake -config release -recursive wolfclient.pro
make dist-gz

cp wolfclient-$VERSION.tar.gz $PKGBUILD/BUILD/.
cd $PKGBUILD/BUILD
tar zxf wolfclient-$VERSION.tar.gz
cd wolfclient-$VERSION

qmake -config release -recursive wolfclient.pro PREFIX=/usr LIBDIR=$LIBDIR
make CXX='ccache g++' PREFIX=/usr LIBDIR=$LIBDIR
# needs X11 or headless X11 server, disabled for now
#make test

make INSTALL_ROOT=$PKGBUILD/PKG install PREFIX=/usr LIBDIR=$LIBDIR

mkdir $PKGBUILD/PKG/install
cp packaging/slackware/slack-desc $PKGBUILD/PKG/install/.
cd $PKGBUILD/PKG
makepkg -l y -c n $PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$ARCH.tgz

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
