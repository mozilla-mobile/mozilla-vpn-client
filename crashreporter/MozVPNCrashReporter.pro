QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

win* {
    message(Windows build)

    TARGET = MozillaVPN

    CONFIG += c++1z
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor

    #Crashpad support
    exists($$PWD/../3rdparty/crashpad/win64/release/include) {
        CONFIG(debug, debug|release) {
            LIBS += -L"$$PWD/../3rdparty/crashpad/win64/release/lib_md"
        }
        CONFIG(release, debug|release) {
            LIBS += -L"$$PWD/../3rdparty/crashpad/win64/release/lib_mt"
        }
        LIBS += -lutil
        LIBS += -lclient
        LIBS += -lbase
    }else{
        error("Crashpad could not be found.  Have you run windows_compile.bat?")
    }

    CONFIG(debug, debug|release) {
        QMAKE_CXXFLAGS += /Z7 /ZI /FdMozVPNCarashReporter.PDB /DEBUG
        QMAKE_LFLAGS_WINDOWS += /DEBUG
    }


}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#translations
RESOURCES += $$PWD/../translations/servers.qrc

exists($$PWD/../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

exists($$PWD/../translations/translations.pri) {
    include($$PWD/../translations/translations.pri)
} else {
    message(Languages were not imported - using fallback English)
    TRANSLATIONS += \
        ../translations/en/mozillavpn_en.ts

    ts.commands += lupdate $$PWD -no-obsolete -ts $$PWD/../translations/en/mozillavpn_en.ts
    ts.CONFIG += no_check_exist
    ts.output = $$PWD/../translations/en/mozillavpn_en.ts
    ts.input = .
    QMAKE_EXTRA_TARGETS += ts
    PRE_TARGETDEPS += ts
}

QMAKE_LRELEASE_FLAGS += -idbased
CONFIG += lrelease
CONFIG += embed_translations
