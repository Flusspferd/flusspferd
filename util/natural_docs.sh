#!/bin/sh

mkdir -p html/docs
NaturalDocs -p naturaldocs -o HTML html/docs -i src/spidermonkey -i src/io \
  -i src/xml -s Default flusspferd $@
