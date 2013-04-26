TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser imageselector formtest

picturechooser.depends = filechooser
