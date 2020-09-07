!versionAtLeast(QT_VERSION, 5.15.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 5.15 or newer")
}

TEMPLATE = subdirs
SUBDIRS += src

linux-g++ {
    SUBDIRS += linux
}
