#!/bin/sh
doxygen help/Doxyfile
./util/jsdocs.sh
groff -man -Thtml build/flusspferd.1 > build/html/flusspferd.1.html
