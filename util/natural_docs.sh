#!/bin/sh
mkdir -p html/docs

if ! which naturaldocs;
then
	ND=NaturalDocs
else
	ND=naturaldocs
fi

$ND -p naturaldocs -o HTML html/docs \
  -i . \
  -xi html \
  -xi include \
  -do -s Default flusspferd

