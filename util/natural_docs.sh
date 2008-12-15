#!/bin/sh

mkdir -p html/docs
#perl -pi -MCwd -e 's!\$pwd!@{[cwd]}!g' naturaldocs/Menu.txt
#tail -n 20 naturaldocs/Menu.txt

NaturalDocs -p naturaldocs -o HTML html/docs \
  -i . \
  -xi html \
  -xi include \
  -do -s Default flusspferd

#tail -n 20 naturaldocs/Menu.txt

#perl -pi -MCwd -e 's!@{[cwd]}!\$pwd!g' naturaldocs/Menu.txt
