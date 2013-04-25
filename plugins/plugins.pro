TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser pictureselector

picturechooser.depends = filechooser
