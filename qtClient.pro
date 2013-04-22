TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = libqtwolframeclient skeleton example plugins qtclient

skeleton.depends = libqtwolframeclient
example.depends = libqtwolframeclient skeleton
qtclient.depends = libqtwolframeclient plugins
