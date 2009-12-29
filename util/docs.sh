#!/bin/sh
doxygen help/Doxyfile

# this is a hack. find sth. better
./util/jsrepl.sh \
    ./plugins/curl/gen-doc.js \
    ./plugins/curl/get_options.cpp \
    > ./plugins/curl/options.pdoc

./util/build_pdocs.rb
rm -f ./plugins/curl/options.pdoc

groff -man -Thtml build/flusspferd.1 > build/html/flusspferd.1.html
