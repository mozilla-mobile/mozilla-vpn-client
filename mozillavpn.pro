!versionAtLeast(QT_VERSION, 5.12.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 5.12 or newer")
}

TEMPLATE = subdirs
SUBDIRS += src

linux {
    SUBDIRS += linux
}
