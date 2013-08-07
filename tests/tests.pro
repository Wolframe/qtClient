TEMPLATE = subdirs

SUBDIRS = SSLavailable datatree datastructmap skeleton xml

test.CONFIG = recursive

QMAKE_EXTRA_TARGETS += test
