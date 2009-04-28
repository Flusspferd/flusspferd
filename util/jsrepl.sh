#!/bin/sh
CMD=$1
[ $# -gt 0 ] && shift
./util/start.sh $CMD ./build/default/src/programs/flusspferd \
	-c ./src/js/jsrepl_dev.js "$@"
