#!/bin/sh

java -cp vendor/jsdoc_toolkit-2.1.0/jsrun.jar \
  JsRun \
  vendor/jsdoc_toolkit-2.1.0/app/run.js \
  -c=jsdoc_template/config.json \
  src/js plugins/ src
