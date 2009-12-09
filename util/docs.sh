#!/bin/sh
doxygen help/Doxyfile
# this is a hack. find sth. better
if [ -f ./src/plugins/curl/gen-doc.js -a -f ./src/plugins/curl/curl.cpp]; then
    ./util/jsrepl.sh ./src/plugins/curl/gen-doc.js ./src/plugins/curl/curl.cpp > ./src/plugins/curl/options.pdoc
fi
./util/build_pdocs.rb
rm -f ./src/plugins/curl/options.pdoc
groff -man -Thtml build/flusspferd.1 > build/html/flusspferd.1.html
