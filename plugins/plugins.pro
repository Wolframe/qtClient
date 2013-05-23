TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser formtest

picturechooser.depends = filechooser
