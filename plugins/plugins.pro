TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = filechooser picturechooser formtest wolfdesigner

picturechooser.depends = filechooser

test.target = test
test.depends = all

QMAKE_EXTRA_TARGETS += test
