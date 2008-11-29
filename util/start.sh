#!/bin/sh
export LD_LIBRARY_PATH=./build/default/src:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=./build/default/src:$DYLD_LIBRARY_PATH
exec "$@"
