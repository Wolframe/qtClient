TEMPLATE = subdirs

SUBDIRS = SSLavailable datatree skeleton xml 
# datastructmap

test.CONFIG = recursive

QMAKE_EXTRA_TARGETS += test
