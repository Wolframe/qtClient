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

cp packaging/solaris/pkginfo $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/solaris/copyright $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/solaris/prototype $PKGBUILD/PKG/wolfclient-$VERSION/.
cp packaging/solaris/depend $PKGBUILD/PKG/wolfclient-$VERSION/.

ARCH=`uname -m`
OSARCH=`uname -p`
OSVER=`uname -r`
cat <<EOF >> $PKGBUILD/PKG/wolfclient-$VERSION/pkginfo
ARCH="$OSARCH"
EOF

cd $PKGBUILD/PKG/wolfclient-$VERSION

pkgmk -o -r .
pkgtrans -s /var/spool/pkg $PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$OSARCH-$OSVER.pkg WFRGclient

compress -f $PKGBUILD/PKGS/$ARCH/wolfclient-$VERSION-$OSARCH-$OSVER.pkg

# rm -rf $PKGBUILD/BUILD/wolfclient-$VERSION
# rm -rf $PKGBUILD/PKG/wolfclient-$VERSION

echo "Build done."
