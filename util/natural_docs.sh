#!/bin/sh
mkdir -p html/docs
if which NaturalDocs >/dev/null
then
	ND=NaturalDocs
else
	ND=naturaldocs
fi

$ND -p naturaldocs -o HTML html/docs -i src/spidermonkey -i src/io \
  -i src/xml -s Default flusspferd $@
