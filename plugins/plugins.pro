TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser wimagelistwidget formtest

picturechooser.depends = filechooser
