TEMPLATE = subdirs

SUBDIRS = SSLavailable datatree skeleton xml
# TEMPORARILY DISABLED datastructmap 

test.CONFIG = recursive

QMAKE_EXTRA_TARGETS += test
