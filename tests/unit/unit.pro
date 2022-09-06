# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += testlib
QT += network
QT += qml
QT += quick
QT += xml
QT += widgets
QT += websockets

DEFINES += BUILD_QMAKE

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
            ../../src/hacl-star/kremlin/minimal

include($$PWD/../../version.pri)
include($$PWD/../../glean/glean.pri)
include($$PWD/../../nebula/nebula.pri)
include($$PWD/../../translations/translations.pri)

# Remove resouce files that we intend to mock out
RESOURCES ~= 's/.*servers.qrc//g'

HEADERS += \
    ../../src/addons/addon.h \
    ../../src/addons/addonapi.h \
    ../../src/addons/addonguide.h \
    ../../src/addons/addoni18n.h \
    ../../src/addons/addonmessage.h \
    ../../src/addons/addonproperty.h \
    ../../src/addons/addonpropertylist.h \
    ../../src/addons/addontutorial.h \
    ../../src/addons/conditionwatchers/addonconditionwatcher.h \
    ../../src/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h \
    ../../src/addons/conditionwatchers/addonconditionwatchergroup.h \
    ../../src/addons/conditionwatchers/addonconditionwatcherjavascript.h \
    ../../src/addons/conditionwatchers/addonconditionwatcherlocales.h \
    ../../src/addons/conditionwatchers/addonconditionwatchertimestart.h \
    ../../src/addons/conditionwatchers/addonconditionwatchertimeend.h \
    ../../src/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h \
    ../../src/addons/manager/addondirectory.h \
    ../../src/addons/manager/addonindex.h \
    ../../src/addons/manager/addonmanager.h \
    ../../src/adjust/adjustfiltering.h \
    ../../src/adjust/adjustproxypackagehandler.h \
    ../../src/captiveportal/captiveportal.h \
    ../../src/collator.h \
    ../../src/command.h \
    ../../src/commandlineparser.h \
    ../../src/composer/composer.h \
    ../../src/composer/composerblock.h \
    ../../src/composer/composerblockbutton.h \
    ../../src/composer/composerblocktext.h \
    ../../src/composer/composerblocktitle.h \
    ../../src/composer/composerblockorderedlist.h \
    ../../src/composer/composerblockunorderedlist.h \
    ../../src/constants.h \
    ../../src/controller.h \
    ../../src/curve25519.h \
    ../../src/dnspingsender.h \
    ../../src/env.h \
    ../../src/errorhandler.h \
    ../../src/externalophandler.h \
    ../../src/frontend/navigator.h \
    ../../src/frontend/navigatorreloader.h \
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
    ../../src/models/featuremodel.h \
    ../../src/models/feedbackcategorymodel.h \
    ../../src/models/keys.h \
    ../../src/models/licensemodel.h \
    ../../src/models/server.h \
    ../../src/models/servercity.h \
    ../../src/models/servercountry.h \
    ../../src/models/servercountrymodel.h \
    ../../src/models/serverdata.h \
    ../../src/models/subscriptiondata.h \
    ../../src/models/supportcategorymodel.h \
    ../../src/models/user.h \
    ../../src/mozillavpn.h \
    ../../src/networkmanager.h \
    ../../src/networkrequest.h \
    ../../src/networkwatcher.h \
    ../../src/networkwatcherimpl.h \
    ../../src/notificationhandler.h \
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
    ../../src/signature.h \
    ../../src/simplenetworkmanager.h \
    ../../src/statusicon.h \
    ../../src/systemtraynotificationhandler.h \
    ../../src/task.h \
    ../../src/tasks/account/taskaccount.h \
    ../../src/tasks/adddevice/taskadddevice.h \
    ../../src/tasks/addon/taskaddon.h \
    ../../src/tasks/function/taskfunction.h \
    ../../src/tasks/ipfinder/taskipfinder.h \
    ../../src/tasks/release/taskrelease.h \
    ../../src/tasks/servers/taskservers.h \
    ../../src/taskscheduler.h \
    ../../src/theme.h \
    ../../src/timersingleshot.h \
    ../../src/tutorial/tutorial.h \
    ../../src/tutorial/tutorialstep.h \
    ../../src/tutorial/tutorialstepbefore.h \
    ../../src/tutorial/tutorialstepnext.h \
    ../../src/update/updater.h \
    ../../src/update/versionapi.h \
    ../../src/update/webupdater.h \
    ../../src/urlopener.h \
    ../../src/websocket/websockethandler.h \
    ../../src/websocket/exponentialbackoffstrategy.h \
    ../../src/websocket/pushmessage.h \
    helper.h \
    testaddon.h \
    testaddonapi.h \
    testaddonindex.h \
    testadjust.h \
    testandroidmigration.h \
    testcommandlineparser.h \
    testcomposer.h \
    testfeature.h \
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
    testurlopener.h \
    websocket/testwebsockethandler.h \
    websocket/testexponentialbackoffstrategy.h \
    websocket/testpushmessage.h

SOURCES += \
    ../../src/addons/addon.cpp \
    ../../src/addons/addonapi.cpp \
    ../../src/addons/addonguide.cpp \
    ../../src/addons/addoni18n.cpp \
    ../../src/addons/addonmessage.cpp \
    ../../src/addons/addonproperty.cpp \
    ../../src/addons/addonpropertylist.cpp \
    ../../src/addons/addontutorial.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatcher.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatchergroup.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatcherjavascript.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatcherlocales.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatchertimestart.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatchertimeend.cpp \
    ../../src/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp \
    ../../src/addons/manager/addondirectory.cpp \
    ../../src/addons/manager/addonindex.cpp \
    ../../src/addons/manager/addonmanager.cpp \
    ../../src/adjust/adjustfiltering.cpp \
    ../../src/adjust/adjustproxypackagehandler.cpp \
    ../../src/captiveportal/captiveportal.cpp \
    ../../src/collator.cpp \
    ../../src/command.cpp \
    ../../src/commandlineparser.cpp \
    ../../src/composer/composer.cpp \
    ../../src/composer/composerblock.cpp \
    ../../src/composer/composerblockbutton.cpp \
    ../../src/composer/composerblocktext.cpp \
    ../../src/composer/composerblocktitle.cpp \
    ../../src/composer/composerblockorderedlist.cpp \
    ../../src/composer/composerblockunorderedlist.cpp \
    ../../src/constants.cpp \
    ../../src/curve25519.cpp \
    ../../src/dnspingsender.cpp \
    ../../src/errorhandler.cpp \
    ../../src/externalophandler.cpp \
    ../../src/frontend/navigator.cpp \
    ../../src/frontend/navigatorreloader.cpp \
    ../../src/hacl-star/Hacl_Chacha20.c \
    ../../src/hacl-star/Hacl_Chacha20Poly1305_32.c \
    ../../src/hacl-star/Hacl_Curve25519_51.c \
    ../../src/hacl-star/Hacl_Poly1305_32.c \
    ../../src/ipaddress.cpp \
    ../../src/ipaddresslookup.cpp \
    ../../src/itempicker.cpp \
    ../../src/inspector/inspectorutils.cpp \
    ../../src/leakdetector.cpp \
    ../../src/localizer.cpp \
    ../../src/logger.cpp \
    ../../src/loghandler.cpp \
    ../../src/models/device.cpp \
    ../../src/models/devicemodel.cpp \
    ../../src/models/feature.cpp \
    ../../src/models/featuremodel.cpp \
    ../../src/models/feedbackcategorymodel.cpp \
    ../../src/models/keys.cpp \
    ../../src/models/licensemodel.cpp \
    ../../src/models/server.cpp \
    ../../src/models/servercity.cpp \
    ../../src/models/servercountry.cpp \
    ../../src/models/servercountrymodel.cpp \
    ../../src/models/serverdata.cpp \
    ../../src/models/subscriptiondata.cpp \
    ../../src/models/supportcategorymodel.cpp \
    ../../src/models/user.cpp \
    ../../src/networkmanager.cpp \
    ../../src/networkwatcher.cpp \
    ../../src/notificationhandler.cpp \
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
    ../../src/signature.cpp \
    ../../src/simplenetworkmanager.cpp \
    ../../src/statusicon.cpp \
    ../../src/tasks/account/taskaccount.cpp \
    ../../src/tasks/adddevice/taskadddevice.cpp \
    ../../src/tasks/addon/taskaddon.cpp \
    ../../src/tasks/function/taskfunction.cpp \
    ../../src/tasks/release/taskrelease.cpp \
    ../../src/tasks/ipfinder/taskipfinder.cpp \
    ../../src/tasks/servers/taskservers.cpp \
    ../../src/taskscheduler.cpp \
    ../../src/theme.cpp \
    ../../src/timersingleshot.cpp \
    ../../src/tutorial/tutorial.cpp \
    ../../src/tutorial/tutorialstep.cpp \
    ../../src/tutorial/tutorialstepbefore.cpp \
    ../../src/tutorial/tutorialstepnext.cpp \
    ../../src/update/updater.cpp \
    ../../src/update/versionapi.cpp \
    ../../src/update/webupdater.cpp \
    ../../src/urlopener.cpp \
    ../../src/websocket/exponentialbackoffstrategy.cpp \
    ../../src/websocket/pushmessage.cpp \
    ../../src/websocket/websockethandler.cpp \
    main.cpp \
    moccontroller.cpp \
    mocinspectorhandler.cpp \
    mocmozillavpn.cpp \
    mocnetworkrequest.cpp \
    mocsystemtraynotificationhandler.cpp \
    testaddon.cpp \
    testaddonapi.cpp \
    testaddonindex.cpp \
    testadjust.cpp \
    testandroidmigration.cpp \
    testcommandlineparser.cpp \
    testcomposer.cpp \
    testfeature.cpp \
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
    testurlopener.cpp \
    websocket/testwebsockethandler.cpp \
    websocket/testexponentialbackoffstrategy.cpp \
    websocket/testpushmessage.cpp

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

RESOURCES += addons/addons.qrc
RESOURCES += servers/servers.qrc
RESOURCES += ../../src/ui/license.qrc
RESOURCES += ../../src/resources/public_keys/public_keys.qrc
RESOURCES += themes/themes.qrc
RESOURCES += guides/guides.qrc
RESOURCES += tutorials/tutorials.qrc
