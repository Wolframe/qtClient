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

QMAKE_EXTRA_TARGETS += test dist-gz dist-Z

PACKAGE_NAME = wolfclient
PACKAGE_VERSION = 0.0.4

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

dist-Z.target = dist-Z
dist-Z.commands = \
	rm -rf $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar.Z ; \
	rm -rf $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	mkdir $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	cp -r * $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	cd $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	gmake distclean ; \
	cd $$TMPDIR ; \
	tar cf $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar $$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	mv $$TMPDIR/$$PACKAGE_NAME-$${PACKAGE_VERSION}.tar $$PWD ; \
	cd $$PWD ; \
	rm -rf $$TMPDIR/$$PACKAGE_NAME-$$PACKAGE_VERSION ; \
	compress $$PACKAGE_NAME-$${PACKAGE_VERSION}.tar
