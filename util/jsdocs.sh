#!/bin/sh

if [ -z "$CLASSPATH" ]; then
  echo Please set CLASSPATH environment variable 1>&2
  exit 1
fi

java JsRun vendor/JSDoc-2.1.0/run.js -c=jsdoc_template/config.json js/src plugins/
