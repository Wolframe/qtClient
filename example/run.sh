#!/bin/sh

LD_LIBRARY_PATH=../libqtwolframeclient:../skeleton
export LD_LIBRARY_PATH

./example $*
