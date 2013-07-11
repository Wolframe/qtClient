TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser formtest wolfdesigner exportplugin

picturechooser.depends = filechooser
