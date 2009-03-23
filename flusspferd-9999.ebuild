# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: ebuild made by Philipp Reh, sefi@s-e-f-i.de$

inherit git

DESCRIPTION="Flusspferd (German for Hippopotamus) is a C++ library providing
Javascript bindings."
HOMEPAGE="https://launchpad.net/flusspferd"

EGIT_REPO_URI="git://github.com/ruediger/flusspferd.git"

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="curl io sandbox sqlite tests xml"

RDEPEND="
	>=dev-lang/spidermonkey-1.7
	>=dev-libs/boost-1.36
	curl ? ( net-misc/curl )
	sqlite ? ( >=dev-db/sqlite-3 )
	xml ? ( dev-libs/libxml2 )"
DEPEND="${RDEPEND}
	dev-lang/python"

src_compile() {
	local options=""

	use curl && options="${options} --enable-curl"
	use io && options="${options} --enable-io"
	use sandbox && options="${options} --enable-sandbox"
	use sqlite && options="${options} --enable-sqlite"
	use tests && options="${options} --enable-tests"
	use xml && options="${options} --enable-xml"

	./waf --with-cxxflags="${CXXFLAGS}" --prefix=/usr ${options} configure || die "waf configure failed"
	./waf build "${MAKEOPTS}" -v || die "waf build failed"
}

src_install() {
	./waf --destdir=${D} install || die "waf install failed"
}
