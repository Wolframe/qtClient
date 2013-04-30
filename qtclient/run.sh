#!/bin/sh

LD_LIBRARY_PATH=../libqtwolframeclient:../skeleton:../plugins/filechooser:../plugins/picturechooser:../demo/crm/client
export LD_LIBRARY_PATH

./qtclient $*
