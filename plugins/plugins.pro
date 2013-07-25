TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser formtest wolfdesigner

picturechooser.depends = filechooser

test.depends = first

QMAKE_EXTRA_TARGETS += test
