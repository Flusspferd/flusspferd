# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: ebuild made by Philipp Reh, sefi@s-e-f-i.de$

inherit git

DESCRIPTION="Flusspferd (German for Hippopotamus) is a C++ library providing
Javascript bindings."
HOMEPAGE="http://flusspferd.org/"

EGIT_REPO_URI="git://github.com/ruediger/flusspferd.git"

EAPI="2"
LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="curl doc gmp libedit readline sqlite tests xml"

RDEPEND="
	>=dev-lang/spidermonkey-1.7[unicode]
	>=dev-libs/boost-1.36
	virtual/libiconv
	curl? ( net-misc/curl )
	gmp? ( dev-libs/gmp[-nocxx] )
	|| (
		libedit? ( >=dev-libs/libedit-20090610.3.0 )
		readline? ( sys-libs/readline )
	)
	sqlite? ( >=dev-db/sqlite-3 )
	xml? ( dev-libs/libxml2 )"
DEPEND="${RDEPEND}
	>=dev-util/cmake-2.6"

src_configure() {
	local options=""

	use libedit && use readline && \
		die "libedit and readline enabled. Please choose just one of them"
	
	use libedit || use readline || options="${options} -D LINE_EDITOR:=none"
	use curl || options="${options} -D PLUGIN_CURL:=OFF"
	use doc && options="${options} -D CREATE_DOCUMENTATION:=ON"
	use gmp || options="${options} -D PLUGIN_GMP:=OFF"
	use libedit && options="${options} -D LINE_EDITOR:=libedit"
	use readline && options="${options} -D LINE_EDITOR:=readline"
	use sqlite || options="${options} -D PLUGIN_SQLITE3:=OFF"
	use tests || options="${options} -D ENABLE_TESTS:=OFF"
	use xml || options="${options} -D PLUGIN_XML:=OFF"

	mkdir build || die
	cd build || die

	cmake \
		-D CMAKE_BUILD_TYPE:=Release \
		-D CMAKE_C_FLAGS:="${CFLAGS}" \
		-D CMAKE_CXX_FLAGS:="${CXXFLAGS}" \
		-D CMAKE_INSTALL_PREFIX=/usr \
		-D FORCE_LINE_EDITOR:=ON \
		-D FORCE_PLUGINS:=ON \
		${options} \
		.. || die "cmake failed"
}

src_compile() {
	cd build
	emake VERBOSE=1 || die "emake failed"
}

src_install() {
	cd build
	emake VERBOSE=1 install DESTDIR="${D}" || die "emake install failed" 
}
