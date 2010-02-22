#!/bin/sh

doxygen=doxygen
builddir=./build
sourcedir=.

# This is a hack because CMake can't set environment vars for commands called via add_custom_command

if [ "$1" ]; then
    doxygen="$1"
fi
if [ "$2" ]; then
    builddir="$2"
fi
if [ "$3" ]; then
    sourcedir="$3"
fi

# this is yet another hack to set the builddir for doxygen
cp help/Doxyfile "$builddir/Doxyfile" && echo "OUTPUT_DIRECTORY       = $builddir" >> "$builddir/Doxyfile" && "$doxygen" "$builddir/Doxyfile"

# this is a hack. find sth. better
"$builddir/bin/flusspferd" -c /dev/null \
    "$sourcedir/plugins/curl/gen-doc.js" \
    "$sourcedir/plugins/curl/get_options.cpp" \
    > "$builddir/options.pdoc"

"$sourcedir/util/build_pdocs.rb" --builddir "$builddir" "$builddir/options.pdoc"

groff -man -Thtml "$builddir/flusspferd.1" > "$builddir/html/flusspferd.1.html"
