#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/bsdbuild
ORIG_ARCH=`uname -m`
if test "x$ORIG_ARCH" = "xamd64"; then
	ARCH="x86_64"
else
if test "x$ORIG_ARCH" = "xi386"; then
	ARCH="i686"
else
	echo "ERROR: Unknown FreeBSD architecture '$ORIG_ARCH'"
	exit 1
fi
fi

rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION

mkdir -p $PKGBUILD $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION $PKGBUILD/PKGS/$ARCH

rm -f $PKGBUILD/BUILD/wolfclient_$VERSION.tar.gz

qmake-qt4 -config release -recursive wolfclient.pro
gmake dist-gz

cp wolfclient-$VERSION.tar.gz $PKGBUILD/BUILD/.
cd $PKGBUILD/BUILD
tar zxf wolfclient-$VERSION.tar.gz
cd wolfclient-$VERSION

qmake-qt4 -config release -recursive wolfclient.pro PREFIX=/usr/local LIBDIR=/usr/local/lib
make CXX='ccache g++' PREFIX=/usr/local LIBDIR=/usr/local/lib
# needs X11 or headless X11 server, disabled for now
#make test

make INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION install PREFIX=/usr/local LIBDIR=/usr/local/lib

cp packaging/freebsd/comment $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/freebsd/description $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/freebsd/packlist $PKGBUILD/PKG/wolfclient-$VERSION/.

cd $PKGBUILD

pkg_create -S $PKGBUILD -z -v \
	-c PKG/wolfclient-$VERSION/comment \
	-d PKG/wolfclient-$VERSION/description \
	-f PKG/wolfclient-$VERSION/packlist \
	$PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$ARCH.tgz

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
