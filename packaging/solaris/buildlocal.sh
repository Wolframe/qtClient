#!/bin/sh

VERSION=0.0.3
PKGBUILD=$HOME/solarisbuild
ORIG_ARCH=`uname -m`

rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION

mkdir -p $PKGBUILD $PKGBUILD/BUILD/wolfclient-$VERSION $PKGBUILD/PKG/wolfclient-$VERSION $PKGBUILD/PKGS/$ARCH

rm -f $PKGBUILD/BUILD/wolfclient_$VERSION.tar.Z

qmake -config release -recursive wolfclient.pro
gmake dist-Z

cp wolfclient-$VERSION.tar.Z $PKGBUILD/BUILD/.
cd $PKGBUILD/BUILD
uncompress -c wolfclient-$VERSION.tar.Z | tar xvf -
cd wolfclient-$VERSION

PATH=/opt/csw/bin:/usr/ccs/bin:/usr/bin:/bin:/usr/local/bin:/opt/csw/sbin:/usr/sbin:/sbin
export PATH
LD_LIBRARY_PATH=/opt/csw/lib:/opt/csw/postgresql/lib
export LD_LIBRARY_PATH

qmake -config release -recursive wolfclient.pro PREFIX=/opt/csw LIBDIR=/opt/csw/lib
for i in `find . -name Makefile`; do
	sed 's|-Wl,-rpath|-Wl,-R|g' $i > /tmp/x; mv -f /tmp/x $i
done
gmake INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION CXX='ccache g++' PREFIX=/opt/csw LIBDIR=/opt/csw/lib
# needs X11 or headless X11 server, disabled for now
#make test

gmake INSTALL_ROOT=$PKGBUILD/PKG/wolfclient-$VERSION install PREFIX=/opt/csw LIBDIR=/opt/csw/lib

#OPSYS=`uname -s`
#OS_VERSION=`uname -r`
#cat <<EOF > $PKGBUILD/PKG/wolfclient-$VERSION/build-info
#MACHINE_ARCH=$PACK_ARCH
#OPSYS=$OPSYS
#OS_VERSION=$OS_VERSION
#PKGTOOLS_VERSION=20091115
#EOF

#cd $PKGBUILD/PKG/wolfclient-$VERSION

#pkg_create -v -p . -I / \
#       -B build-info \
#       -c comment \
#       -d description \
#       -f packlist \
#       -i iscript \
#       -k dscript \
#       $PKGBUILD/PKGS/$ARCH/wolframe-$VERSION-$ARCH.tgz
#check_for_errors

# rm -rf $PKGBUILD/BUILD
# rm -rf $PKGBUILD/PKG

echo "Build done."
