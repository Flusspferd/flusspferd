#!/bin/sh
doxygen help/Doxyfile
./util/build_pdocs.rb
groff -man -Thtml build/flusspferd.1 > build/html/flusspferd.1.html
