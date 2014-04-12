#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/bsdbuild
ORIG_ARCH=`uname -m`
ORIG_OS_VER=`uname -r | cut -f 1 -d -`
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

# use Qt 5 on FreeBSD 10.0, Qt 4 on 9 and 8
if test "x$ORIG_OS_VER" = "x10.0"; then
	QMAKE=/usr/local/lib/qt5/bin/qmake
	QMAKE_CXX='ccache clang++'
else
	QMAKE=qmake-qt4
	QMAKE_CXX='ccache g++'
fi

rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION

mkdir -p $PKGBUILD $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION $PKGBUILD/PKGS/$ARCH

rm -f $PKGBUILD/BUILD/wolfclient_$VERSION.tar.gz

$QMAKE -config release -recursive wolfclient.pro
gmake dist-gz

cp wolfclient-$VERSION.tar.gz $PKGBUILD/BUILD/.
cd $PKGBUILD/BUILD
tar zxf wolfclient-$VERSION.tar.gz
cd wolfclient-$VERSION

$QMAKE -config release -recursive wolfclient.pro PREFIX=/usr/local LIBDIR=/usr/local/lib QMAKE_CXX="$QMAKE_CXX"
make CXX="$QMAKE_CXX" PREFIX=/usr/local LIBDIR=/usr/local/lib
# needs X11 or headless X11 server, disabled for now
#make test

make INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION install PREFIX=/usr/local LIBDIR=/usr/local/lib

# pkgng for 10, png_create for 9 and 8
if test "x$ORIG_OS_VER" = "x10.0"; then
	cp packaging/freebsd/+MANIFEST $PKGBUILD/PKG/.
	cd $PKGBUILD
	pkg create -o $PKGBUILD/PKGS/$ARCH -m PKG -r PKG/wolfclient-$VERSION
else
	cp packaging/freebsd/comment $PKGBUILD/PKG/wolfclient-$VERSION/.
	cp packaging/freebsd/description $PKGBUILD/PKG/wolfclient-$VERSION/.
	cp packaging/freebsd/packlist $PKGBUILD/PKG/wolfclient-$VERSION/.
	cd $PKGBUILD
	pkg_create -S $PKGBUILD -z -v \
		-c PKG/wolfclient-$VERSION/comment \
		-d PKG/wolfclient-$VERSION/description \
		-f PKG/wolfclient-$VERSION/packlist \
		$PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$ARCH.tgz
fi

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
