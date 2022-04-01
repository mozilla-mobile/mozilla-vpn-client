# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network
QT += qml
QT += quick
QT += xml
QT += widgets

DEFINES += APP_VERSION=\\\"1234\\\"
DEFINES += BUILD_ID=\\\"1234\\\"

CONFIG += c++1z

macos {
    CONFIG -= app_bundle
}

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00
DEFINES += UNIT_TEST
DEFINES += MVPN_ADJUST

TEMPLATE = app
TARGET = tests

INCLUDEPATH += \
            . \
            ../../src \
            ../../src/hacl-star \
            ../../src/hacl-star/kremlin \
            ../../src/hacl-star/kremlin/minimal \
            ../../translations/generated \
            ../../glean \
            ../../nebula

include($$PWD/../../glean/glean.pri)
include($$PWD/../../nebula/nebula.pri)

HEADERS += \
    ../../src/adjust/adjustfiltering.h \
    ../../src/adjust/adjustproxypackagehandler.h \
    ../../src/captiveportal/captiveportal.h \
    ../../src/collator.h \
    ../../src/command.h \
    ../../src/commandlineparser.h \
    ../../src/constants.h \
    ../../src/controller.h \
    ../../src/curve25519.h \
    ../../src/dnspingsender.h \
    ../../src/errorhandler.h \
    ../../src/featurelist.h \
    ../../src/inspector/inspectorhandler.h \
    ../../src/inspector/inspectorutils.h \
    ../../src/ipaddress.h \
    ../../src/ipaddresslookup.h \
    ../../src/itempicker.h \
    ../../src/leakdetector.h \
    ../../src/localizer.h \
    ../../src/logger.h \
    ../../src/loghandler.h \
    ../../src/models/device.h \
    ../../src/models/devicemodel.h \
    ../../src/models/feature.h \
    ../../src/models/feedbackcategorymodel.h \
    ../../src/models/guide.h \
    ../../src/models/guideblock.h \
    ../../src/models/guidemodel.h \
    ../../src/models/helpmodel.h \
    ../../src/models/keys.h \
    ../../src/models/licensemodel.h \
    ../../src/models/server.h \
    ../../src/models/servercity.h \
    ../../src/models/servercountry.h \
    ../../src/models/servercountrymodel.h \
    ../../src/models/serverdata.h \
    ../../src/models/supportcategorymodel.h \
    ../../src/models/survey.h \
    ../../src/models/surveymodel.h \
    ../../src/models/user.h \
    ../../src/models/whatsnewmodel.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/networkwatcher.h \
    ../../src/networkwatcherimpl.h \
    ../../src/pinghelper.h \
    ../../src/pingsender.h \
    ../../src/pingsenderfactory.h \
    ../../src/platforms/android/androiddatamigration.h \
    ../../src/platforms/android/androidsharedprefs.h \
    ../../src/platforms/dummy/dummynetworkwatcher.h \
    ../../src/platforms/dummy/dummypingsender.h \
    ../../src/qmlengineholder.h \
    ../../src/releasemonitor.h \
    ../../src/rfc/rfc1918.h \
    ../../src/rfc/rfc4193.h \
    ../../src/rfc/rfc4291.h \
    ../../src/rfc/rfc5735.h \
    ../../src/serveri18n.h \
    ../../src/settingsholder.h \
    ../../src/simplenetworkmanager.h \
    ../../src/statusicon.h \
    ../../src/task.h \
    ../../src/tasks/account/taskaccount.h \
    ../../src/tasks/adddevice/taskadddevice.h \
    ../../src/tasks/ipfinder/taskipfinder.h \
    ../../src/tasks/function/taskfunction.h \
    ../../src/tasks/release/taskrelease.h \
    ../../src/tasks/servers/taskservers.h \
    ../../src/taskscheduler.h \
    ../../src/theme.h \
    ../../src/timersingleshot.h \
    ../../src/tutorial.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/urlopener.h \
    helper.h \
    testadjust.h \
    testandroidmigration.h \
    testcommandlineparser.h \
    testfeature.h \
    testguide.h \
    testipaddress.h \
    testipaddresslookup.h \
    testipfinder.h \
    testlicense.h \
    testlocalizer.h \
    testlogger.h \
    testmodels.h \
    testmozillavpnh.h \
    testnetworkmanager.h \
    testreleasemonitor.h \
    testserveri18n.h \
    teststatusicon.h \
    testtasks.h \
    testthemes.h \
    testtimersingleshot.h \
    testtutorial.h

SOURCES += \
    ../../src/adjust/adjustfiltering.cpp \
    ../../src/adjust/adjustproxypackagehandler.cpp \
    ../../src/captiveportal/captiveportal.cpp \
    ../../src/collator.cpp \
    ../../src/command.cpp \
    ../../src/commandlineparser.cpp \
    ../../src/constants.cpp \
    ../../src/curve25519.cpp \
    ../../src/dnspingsender.cpp \
    ../../src/errorhandler.cpp \
    ../../src/featurelist.cpp \
    ../../src/hacl-star/Hacl_Chacha20.c \
    ../../src/hacl-star/Hacl_Chacha20Poly1305_32.c \
    ../../src/hacl-star/Hacl_Curve25519_51.c \
    ../../src/hacl-star/Hacl_Poly1305_32.c \
    ../../src/ipaddress.cpp \
    ../../src/ipaddresslookup.cpp \
    ../../src/itempicker.cpp \
    ../../src/inspector/inspectorutils.cpp \
    ../../src/l18nstringsimpl.cpp \
    ../../src/leakdetector.cpp \
    ../../src/localizer.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/device.cpp \
    ../../src/models/devicemodel.cpp \
    ../../src/models/feature.cpp \
    ../../src/models/feedbackcategorymodel.cpp \
    ../../src/models/guide.cpp \
    ../../src/models/guideblock.cpp \
    ../../src/models/guidemodel.cpp \
    ../../src/models/helpmodel.cpp \
    ../../src/models/keys.cpp \
    ../../src/models/licensemodel.cpp \
    ../../src/models/server.cpp \
    ../../src/models/servercity.cpp \
    ../../src/models/servercountry.cpp \
    ../../src/models/servercountrymodel.cpp \
    ../../src/models/serverdata.cpp \
    ../../src/models/supportcategorymodel.cpp \
    ../../src/models/survey.cpp \
    ../../src/models/surveymodel.cpp \
    ../../src/models/user.cpp \
    ../../src/models/whatsnewmodel.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkwatcher.cpp \
    ../../src/pinghelper.cpp \
    ../../src/pingsenderfactory.cpp \
    ../../src/platforms/android/androiddatamigration.cpp \
    ../../src/platforms/android/androidsharedprefs.cpp \
    ../../src/platforms/dummy/dummynetworkwatcher.cpp \
    ../../src/platforms/dummy/dummypingsender.cpp \
    ../../src/qmlengineholder.cpp \
    ../../src/releasemonitor.cpp \
    ../../src/rfc/rfc1918.cpp \
    ../../src/rfc/rfc4193.cpp \
    ../../src/rfc/rfc4291.cpp \
    ../../src/rfc/rfc5735.cpp \
    ../../src/serveri18n.cpp \
    ../../src/settingsholder.cpp \
    ../../src/simplenetworkmanager.cpp \
    ../../src/statusicon.cpp \
    ../../src/tasks/account/taskaccount.cpp \
    ../../src/tasks/adddevice/taskadddevice.cpp \
    ../../src/tasks/ipfinder/taskipfinder.cpp \
    ../../src/tasks/function/taskfunction.cpp \
    ../../src/tasks/release/taskrelease.cpp \
    ../../src/tasks/servers/taskservers.cpp \
    ../../src/taskscheduler.cpp \
    ../../src/theme.cpp \
    ../../src/timersingleshot.cpp \
    ../../src/tutorial.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/urlopener.cpp \
    main.cpp \
    moccontroller.cpp \
    mocinspectorhandler.cpp \
    mocmozillavpn.cpp \
    mocnetworkrequest.cpp \
    testadjust.cpp \
    testandroidmigration.cpp \
    testcommandlineparser.cpp \
    testfeature.cpp \
    testguide.cpp \
    testipaddress.cpp \
    testipaddresslookup.cpp \
    testipfinder.cpp \
    testlicense.cpp \
    testlocalizer.cpp \
    testlogger.cpp \
    testmodels.cpp \
    testmozillavpnh.cpp \
    testnetworkmanager.cpp \
    testreleasemonitor.cpp \
    testserveri18n.cpp \
    teststatusicon.cpp \
    testtasks.cpp \
    testthemes.cpp \
    testtimersingleshot.cpp \
    testtutorial.cpp

exists($$PWD/../../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

# Platform-specific: Linux
linux {
    # QMAKE_CXXFLAGS *= -Werror
}

# Platform-specific: MacOS
else:macos {
    # For the loginitem
    LIBS += -framework ServiceManagement
    LIBS += -framework Security

    # QMAKE_CXXFLAGS *= -Werror

    OBJECTIVE_SOURCES += \
            ../../src/platforms/macos/macosutils.mm

    OBJECTIVE_HEADERS += \
            ../../src/platforms/macos/macosutils.h
}

# Platform-specific: IOS
else:ios {
    DEFINES += MVPN_IOS

    OBJECTIVE_SOURCES += \
            ../../src/platforms/ios/iosutils.mm

    OBJECTIVE_HEADERS += \
            ../../src/platforms/ios/iosutils.h
}

# Platform-specific: windows
else:win* {
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

RESOURCES += servers/servers.qrc
RESOURCES += ../../src/ui/license.qrc
RESOURCES += themes/themes.qrc
RESOURCES += guides/guides.qrc
