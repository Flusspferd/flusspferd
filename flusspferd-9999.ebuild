# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: ebuild made by Philipp Reh, sefi@s-e-f-i.de$

inherit git

DESCRIPTION="Flusspferd (German for Hippopotamus) is a C++ library providing
Javascript bindings."
HOMEPAGE="http://flusspferd.org/"

EGIT_REPO_URI="git://github.com/ruediger/flusspferd.git"

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

RDEPEND="
	>=dev-lang/spidermonkey-1.7
	>=dev-libs/boost-1.36"
DEPEND="${RDEPEND}
	dev-lang/python"

src_compile() {
	./waf --with-cxxflags="${CXXFLAGS}" --prefix=/usr configure || die "waf configure failed"
	./waf build "${MAKEOPTS}" -v || die "waf build failed"
}

src_install() {
	./waf --destdir=${D} install || die "waf install failed"
}
