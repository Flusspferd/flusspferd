#!/bin/sh
doxygen help/Doxyfile

# this is a hack. find sth. better
./util/jsrepl.sh \
    ./src/plugins/curl/gen-doc.js \
    ./src/plugins/curl/get_options.cpp \
    > ./src/plugins/curl/options.pdoc

./util/build_pdocs.rb
rm -f ./src/plugins/curl/options.pdoc

groff -man -Thtml build/flusspferd.1 > build/html/flusspferd.1.html
