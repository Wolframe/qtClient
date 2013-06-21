# Contributor: Andreas Baumann <abaumann at yahoo dot com>
# Maintainer: Andreas Baumann <abaumann at yahoo dot com>
pkgname=wolframe-qtclient-git
pkgver=0.0.1
pkgrel=1
pkgdesc="Qt client for Wolframe."
license=('GPL3')
arch=('i686' 'x86_64')
url="http://wolframe.net/"
depends=('qt4')
makedepends=('git')

_gitroot=git://github.com/Wolframe/qtClient.git
_gitname=qtClient

package() {
  cd ${srcdir}/$_gitname-build

  msg "Installing.."
  make install INSTALL_ROOT=${pkgdir}
}

build() {
  cd ${srcdir}
  
  msg "Getting source from git..."
  if [ -d ${srcdir}/$_gitname ] ; then
    cd $_gitname && git pull origin
  else
    git clone $_gitroot
  fi

  cp -r ${srcdir}/$_gitname ${srcdir}/$_gitname-build
  cd ${srcdir}/$_gitname-build

  msg "Generating makefiles.."
  qmake-qt4 PREFIX=/usr
  
  msg "Building..."
  make
}
