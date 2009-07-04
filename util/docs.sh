#!/bin/sh
doxygen help/Doxyfile
./util/jsdocs.sh
groff -man -Thtml help/flusspferd.1 > build/html/flusspferd.1.html
