#!/bin/sh

LD_LIBRARY_PATH=../../libqtwolfclient:../../skeleton
export LD_LIBRARY_PATH

./skeletonTest $*
