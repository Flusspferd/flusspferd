#!/bin/sh

# this is a small hack to make running tests from CTest easier.

if [ -d "$1" ]; then
    cd "$1"
    shift # remove $1 from $@
fi

./util/jsrepl.sh "$@"
