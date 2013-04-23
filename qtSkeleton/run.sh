#!/bin/sh

LD_LIBRARY_PATH=../libqtwolframeclient:../skeleton:../plugins/filechooser:../plugins/picturechooser
export LD_LIBRARY_PATH

./qtSkeleton $*
