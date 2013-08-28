TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = libqtwolfclient skeleton example plugins wolfclient tests

skeleton.depends = libqtwolfclient
example.depends = libqtwolfclient skeleton
wolfclient.depends = libqtwolfclient plugins
tests.depends = wolfclient

OTHER_FILES += 

test.CONFIG = recursive
test.depends = first

QMAKE_EXTRA_TARGETS += test
