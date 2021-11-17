QT -= gui
QT += qml
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++1z

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    nebula.cpp

HEADERS += \
    nebula.h

RESOURCES += ./ui/components.qrc
RESOURCES += ./ui/themes.qrc
RESOURCES += ./ui/nebula_resources.qrc

QML_IMPORT_PATH+=$$PWD/ui


versionAtLeast(QT_VERSION, 6.0.0) {
    RESOURCES += ui/compatQt6.qrc
    RESOURCES += ui/resourcesQt6.qrc
} else {
    RESOURCES += ui/compatQt5.qrc
}
CONFIG(debug, debug|release) {
    DESTDIR=debug
}
CONFIG(release, debug|release) {
    DESTDIR=release
}
android {
    CONFIG(debug, debug|release) {
        DESTDIR=$$ANDROID_TARGET_ARCH/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR=$$ANDROID_TARGET_ARCH/release
    }
}
