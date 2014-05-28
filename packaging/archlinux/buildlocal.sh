#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/archbuild
ARCH=`uname -m`
LIBDIR="/usr/lib"
rm -rf $PKGBUILD/BUILD $PKGBUILD/PKG

check_for_errors( )
{
	RET=$?
	if test $RET -gt 0; then
		echo "Build failed."
		exit 1
	fi
}

mkdir -p $PKGBUILD $PKGBUILD/BUILD $PKGBUILD/PKG $PKGBUILD/PKGS/$ARCH

rm -f wolfclient-$VERSION.tar.gz

make distclean
mkdir /tmp/wolfclient-$VERSION
cp -a * /tmp/wolfclient-$VERSION
cd /tmp
tar zcf wolfclient-$VERSION.tar.gz wolfclient-$VERSION
cd -
mv /tmp/wolfclient-$VERSION.tar.gz .
rm -rf /tmp/wolfclient-$VERSION

cp wolfclient-$VERSION.tar.gz $PKGBUILD/BUILD/.
cp packaging/archlinux/* $PKGBUILD/BUILD/.
sed -i 's|\(qmake\-qt4.*\)|\1 QMAKE_CXX="\$CXX" QMAKE_LINK=\"$CXX\" QMAKE_LINK_SHLIB=\"$CXX\"|g' $PKGBUILD/BUILD/PKGBUILD

if test -x /etc/profile.d/intel_compilers.sh; then
	MACHINE_ARCH=`uname -m`
	if test "$MACHINE_ARCH" = "x86_64"; then
		export QMAKESPEC="linux-icc-64"
	else
		if test "$MACHINE_ARCH" = "i686"; then
			export QMAKESPEC="linux-icc-32"
		else
			print "ERROR: Unknown Intel architecture $MACHINE_ARCH!"
			exit 1
		fi
	fi
	. /etc/profile.d/intel_compilers.sh
	export QMAKE_CXX='ccache icpc'
fi

cd $PKGBUILD/BUILD
sed -i 's|"http://sourceforge.net/projects/wolframe/files/.*"|"${pkgname}-${pkgver}.tar.gz"|g' PKGBUILD
makepkg --asroot -g >> PKGBUILD
makepkg --asroot 
check_for_errors

cp $PKGBUILD/BUILD/wolfclient-*.pkg.tar.xz $PKGBUILD/PKGS/$ARCH/.

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build succeeded."
exit 0
