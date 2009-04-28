#!/bin/sh
CMD=$1
[ $# -gt 0 ] && shift
./util/start.sh $CMD ./build/default/programs/flusspferd \
	-c ./src/js/jsrepl_dev.js "$@"
