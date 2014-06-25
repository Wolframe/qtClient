# Contributor: Andreas Baumann <abaumann at yahoo dot com>
# Maintainer: Andreas Baumann <abaumann at yahoo dot com>
pkgname=wolfclient-git
pkgver=release_0.0.4.r7.gd805045
pkgrel=1
pkgdesc="Client for Wolframe."
license=('GPL3')
arch=('i686' 'x86_64' 'armv6h')
url="http://wolframe.net/"
depends=('qt4')
makedepends=('git' 'mesa-libgl' 'gdb')
conflicts=('wolfclient')
provides=('wolfclient')
source=('git://github.com/Wolframe/wolfclient.git')
md5sums=('SKIP')

pkgver() {
   cd ${srcdir}/wolfclient
   git describe --long --tags | sed -r 's/([^-]*-g)/r\1/;s/-/./g'
}

package() {
  cd ${srcdir}/wolfclient

  # regenarete Makefiles here, as files have to exist in order for
  # the install rules to be generated
  qmake-qt4 -config debug -recursive PREFIX=/usr LIBDIR=/usr/lib/wolframe

  msg "Installing.."
  make install INSTALL_ROOT=${pkgdir}
}

build() {
  cd ${srcdir}/wolfclient
  
  msg "Generating makefiles.."
  qmake-qt4 -config debug -recursive PREFIX=/usr LIBDIR=/usr/lib/wolframe
  
  msg "Building..."
  make
}

check() {
  cd ${srcdir}/wolfclient

  msg "Testing..."
  make test
}
