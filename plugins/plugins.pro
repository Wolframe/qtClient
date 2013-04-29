TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser imagelistview formtest

picturechooser.depends = filechooser
