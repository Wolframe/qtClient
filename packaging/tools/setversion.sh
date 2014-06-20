#!/bin/sh

if test $# != 1; then
	echo "usage: setversion.sh <VERSION>"
	exit 1
fi

FULL_VERSION=$1

MAJOR_VERSION=`echo $FULL_VERSION | cut -f 1 -d .`
MINOR_VERSION=`echo $FULL_VERSION | cut -f 2 -d .`
REVISION=`echo $FULL_VERSION | cut -f 3 -d .`
BUILD=`echo $FULL_VERSION | cut -f 4 -d .`

VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REVISION"

# Set PACKAGE_VERSION in master PRO file (make dist and building)
sed -i "s/^PACKAGE_VERSION.*=.*/PACKAGE_VERSION = $VERSION/g" wolfclient.pro

# WOLFRAME_VERSION constant
sed -i "s/^#define WOLFRAME_VERSION.*/#define WOLFRAME_VERSION \"$VERSION\"/g" wolfclient/global.hpp

# package metadata
sed -i "s/pkgver=.*/pkgver=$VERSION/g" packaging/archlinux/PKGBUILD
sed -i "s/^Version:.*/Version: $VERSION/g" packaging/redhat/wolfclient.spec
sed -i "s/^wolfclient-.*:/wolfclient-$VERSION:/g" packaging/slackware/slack-desc
sed -i "s/<?define.*ProductVersion.*=.*\"[^\"]*\".*?>/<?define ProductVersion = \"$VERSION\" ?>/g" packaging/windows/wolfclient.wxs
sed -i "s/^VERSION=.*/VERSION=\"$VERSION\"/g" packaging/solaris/pkginfo
sed -i "s/^version:.*/version: $VERSION/g" packaging/freebsd/+MANIFEST
sed -i "s/^wolfclient - Wolframe client, .*/wolfclient - Wolframe client, $VERSION/g" packaging/freebsd/comment
sed -i "s/^@name wolfclient-.*/@name wolfclient-$VERSION/g" packaging/freebsd/packlist
sed -i "s/^@srcdir PKG\/wolfclient-.*/@srcdir PKG\/wolfclient-$VERSION/g" packaging/freebsd/packlist
sed -i "s/^wolfclient - Wolframe client, .*/wolfclient - Wolframe client, $VERSION/g" packaging/netbsd/comment
sed -i "s/^@name wolfclient-.*/@name wolfclient-$VERSION/g" packaging/netbsd/packlist
sed -i "s/^@srcdir PKG\/wolfclient-.*/@srcdir PKG\/wolfclient-$VERSION/g" packaging/netbsd/packlist

# local package build scripts
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/redhat/buildlocal.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/archlinux/buildlocal.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/slackware/buildlocal.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/solaris/buildlocal.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/freebsd/buildlocal.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/netbsd/buildlocal.sh

# OpenSuse build
sed -i "s/pkgver=.*/pkgver=$VERSION/g" packaging/obs/PKGBUILD
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/obs/release_osb_binaries.sh
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/obs/deploy_to_obs.sh
sed -i "s/^Version: [^\-]*/Version: $VERSION/g" packaging/obs/wolfclient.dsc

# package releasing
sed -i "s/^VERSION=.*/VERSION=$VERSION/g" packaging/obs/release_local_binaries.sh
