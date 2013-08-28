#!/bin/sh

LD_LIBRARY_PATH=../libqtwolfclient:../skeleton:../plugins/filechooser:../plugins/picturechooser
export LD_LIBRARY_PATH

./qtclient $*
