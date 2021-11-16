QT -= gui
QT += qml
TEMPLATE = lib

!android{
CONFIG += staticlib
}

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

android {
QMAKE_LINK += -nostdlib++
equals(ANDROID_TARGET_ARCH,arm64-v8a) {
        LIBS += -L$$PWD/../Libs/android_arm64-v8a/ -lLibrary
        ANDROID_EXTRA_LIBS += $$PWD/../Libs/android_arm64-v8a/nebula.so
}
equals(ANDROID_TARGET_ARCH,armeabi-v7a) {
        LIBS += -L$$PWD/../Libs/android_armeabi-v7a/ -lLibrary
        ANDROID_EXTRA_LIBS += $$PWD/../Libs/android_armeabi-v7a/nebula.so
}
}
