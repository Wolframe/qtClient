TEMPLATE = subdirs

SUBDIRS = SSLavailable functions skeleton

test.target = test
test.CONFIG = recursive

QMAKE_EXTRA_TARGETS += test
