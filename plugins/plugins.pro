TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser imageselector

picturechooser.depends = filechooser
