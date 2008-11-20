#!/bin/sh
export LD_LIBRARY_PATH=./build/default/src:$LD_LIBRARY_PATH
exec "$@"
