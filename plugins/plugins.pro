TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser formtest wolfdesigner

picturechooser.depends = filechooser
