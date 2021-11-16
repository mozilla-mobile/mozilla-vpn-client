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

exists($$PWD/telemetry/gleansample.h) {
    HEADERS+=$$PWD/telemetry/gleansample.h
}
else{
    error(Glean generated files are missing. Please run `python3 ./scripts/generate_glean.py`)
}

SOURCES += \
    glean.cpp

HEADERS += \
    glean.h

!wasm {
    message(Include QSQlite plugin)
    QTPLUGIN += qsqlite
}

QML_IMPORT_PATH+=$$PWD
RESOURCES += glean.qrc

android {
QMAKE_LINK += -nostdlib++
equals(ANDROID_TARGET_ARCH,arm64-v8a) {
        LIBS += -L$$PWD/../Libs/android_arm64-v8a/ -lLibrary
        ANDROID_EXTRA_LIBS += $$PWD/../Libs/android_arm64-v8a/glean.so
}
equals(ANDROID_TARGET_ARCH,armeabi-v7a) {
        LIBS += -L$$PWD/../Libs/android_armeabi-v7a/ -lLibrary
        ANDROID_EXTRA_LIBS += $$PWD/../Libs/android_armeabi-v7a/glean.so
}
}
