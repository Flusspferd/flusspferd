#!/bin/sh
CMD=$1
[ $# -gt 0 ] && shift
./util/start.sh $CMD ./build/default/sandbox/jsrepl "$@"
