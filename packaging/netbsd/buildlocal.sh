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
	echo "ERROR: Unknown NetBSD architecture '$ORIG_ARCH'"
	exit 1
fi
fi

rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION

mkdir -p $PKGBUILD $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION $PKGBUILD/PKGS/$ARCH

rm -f wolfclient-$VERSION.tar.gz
rm -f $PKGBUILD/BUILD/wolfclient_$VERSION.tar.gz

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

export PATH=/usr/pkg/qt4/bin:/usr/pkg/bin:${PATH}
export QTDIR=/usr/pkg/qt4

qmake -config release -recursive wolfclient.pro PREFIX=/usr/pkg LIBDIR=/usr/pkg/lib
find . -name Makefile -exec sh -c \
	"sed 's/libtool --silent/libtool --silent --tag=CXX/g' {} > x && mv x {}" \;
make INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION CXX='ccache g++' PREFIX=/usr/pkg LIBDIR=/usr/pkg/lib
# needs X11 or headless X11 server, disabled for now
#make test

make INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION install PREFIX=/usr/pkg LIBDIR=/usr/pkg/lib

# some cleanup after libtool
rm -f $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/libqtwolfclient.la
rm -f $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/libqtwolfclient.so
rm -f $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/libskeleton.la
rm -f $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/libskeleton.so
rm -f $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/qt4/plugins/designer/*.la
# relinking libtool issue, copy library manually and hoping the best..
cp -a $PKGBUILD/BUILD/wolfclient-$VERSION/skeleton/.libs/libskeleton.so.* $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/.
cp -a $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/qt4/plugins/designer/*.so $PKGBUILD/PKG/wolfclient-$VERSION/usr/pkg/lib/.

cp packaging/netbsd/comment $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/netbsd/description $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/netbsd/packlist $PKGBUILD/PKG/wolfclient-$VERSION/.

OPSYS=`uname -s`
OS_VERSION=`uname -r`
cat <<EOF > $PKGBUILD/PKG/wolfclient-$VERSION/build-info
MACHINE_ARCH=$ORIG_ARCH
OPSYS=$OPSYS
OS_VERSION=$OS_VERSION
PKGTOOLS_VERSION=20091115
EOF

cd $PKGBUILD/PKG/wolfclient-0.0.3

pkg_create -v -p .  -I / \
	-B build-info \
	-c comment \
	-d description \
	-f packlist \
	$PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$ARCH.tgz

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
