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
	echo "ERROR: Unknown slackware architecture '$ARCH'"
	exit 1
fi
fi

rm -rf $PKGBUILD/BUILD $PKGBUILD/PKG

mkdir -p $PKGBUILD $PKGBUILD/BUILD $PKGBUILD/PKG $PKGBUILD/PKGS/$ARCH

rm -f wolfclient-$VERSION.tar.gz
rm -f $RPMBUILD/BUILD/wolfclient_$VERSION.tar.gz

gmake distclean
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

qmake-qt4 -config release -recursive wolfclient.pro PREFIX=/usr/local LIBDIR=/usr/local/lib
make CXX='ccache g++' PREFIX=/usr/local LIBDIR=/usr/local/lib
# needs X11 or headless X11 server, disabled for now
#make test

find . -name Makefile -exec rm -f {} \;
qmake-qt4 -config release -recursive wolfclient.pro PREFIX=/usr/local LIBDIR=/usr/local/lib
make INSTALL_ROOT=$PKGBUILD/PKG install PREFIX=/usr/local LIBDIR=/usr/local/lib

cp packaging/freebsd/comment $PKGBUILD/PKG/.
cp packaging/freebsd/description $PKGBUILD/PKG/.
cp packaging/freebsd/packlist $PKGBUILD/PKG/.

cd $PKGBUILD

pkg_create -S $PKGBUILD -z -v \
	-c PKG/comment \
	-d PKG/description \
	-f PKG/packlist \
	"wolfclient-$VERSION"

mv $PKGBUILD/wolfclient-$VERSION.tgz $PKGBUILD/PKGS/$ARCH/.	

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
