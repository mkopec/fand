# Maintainer: Your Name <tomasz.pakula.external@atos.net>
pkgname=(fand)
pkgver=1.3
pkgrel=1
pkgdesc="A tool for managing your PC's fans"
arch=(x86_64)
url="https://github.com/mkopec/fand"
license=('GPL')
makedepends=(gcc)
provides=(fand)
conflicts=(fand-snapshot)
source=(*)
sha256sums=('SKIP')

#prepare() {
#
#}

build() {
  make
}

package() {
  mkdir -p "$pkgdir/usr/bin/"
  mv main "$pkgdir/usr/bin/fand"
}
