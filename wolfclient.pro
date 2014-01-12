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

QMAKE_EXTRA_TARGETS += test dist-gz

PACKAGE_NAME = wolfclient
PACKAGE_VERSION = 0.0.3

isEmpty(TMPDIR) {
  TMPDIR = $${PREFIX}/tmp
}

dist-gz.target = dist-gz
dist-gz.commands = \
	rm -rf $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar.gz ; \
	rm -rf $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	mkdir $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	cp -a * $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	cd $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	make distclean ; \
	cd $$TMPDIR ; \
	tar cf $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar $$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	mv $$TMPDIR/$$PACKAGE_NAME-$${PACKAGE_VERSION}.tar $$PWD ; \
	cd $$PWD ; \
	rm -rf $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	gzip -9 $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar
