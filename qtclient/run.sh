#!/bin/sh

LD_LIBRARY_PATH=../libqtwolframeclient:../skeleton:../plugins/filechooser:../plugins/picturechooser:../plugins/wimagelistwidget
export LD_LIBRARY_PATH

./qtclient $*
