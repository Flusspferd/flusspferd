#!/bin/sh
tidy --output-xhtml y --indent "auto" --indent-spaces "2" --wrap "90" --char-encoding "utf8" index.html >index.html.new
mv index.html.new index.html
