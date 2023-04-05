# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += \
        $$PWD/addons/addon.cpp \
        $$PWD/addons/addonapi.cpp \
        $$PWD/addons/addonguide.cpp \
        $$PWD/addons/addoni18n.cpp \
        $$PWD/addons/addonmessage.cpp \
        $$PWD/addons/addonproperty.cpp \
        $$PWD/addons/addonpropertylist.cpp \
        $$PWD/addons/addontutorial.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatcher.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatchergroup.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatcherjavascript.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatcherlocales.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatchertime.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp \
        $$PWD/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp \
        $$PWD/addons/manager/addondirectory.cpp \
        $$PWD/addons/manager/addonindex.cpp \
        $$PWD/addons/manager/addonmanager.cpp \
        $$PWD/addons/state/addonsessionstate.cpp \
        $$PWD/addons/state/addonstatebase.cpp \
        $$PWD/app.cpp \
        $$PWD/authenticationlistener.cpp \
        $$PWD/authenticationinapp/authenticationinapp.cpp \
        $$PWD/authenticationinapp/authenticationinapplistener.cpp \
        $$PWD/authenticationinapp/authenticationinappsession.cpp \
        $$PWD/authenticationinapp/incrementaldecoder.cpp \
        $$PWD/collator.cpp \
        $$PWD/composer/composer.cpp \
        $$PWD/composer/composerblock.cpp \
        $$PWD/composer/composerblockbutton.cpp \
        $$PWD/composer/composerblocktext.cpp \
        $$PWD/composer/composerblocktitle.cpp \
        $$PWD/composer/composerblockorderedlist.cpp \
        $$PWD/composer/composerblockunorderedlist.cpp \
        $$PWD/constants.cpp \
        $$PWD/cryptosettings.cpp \
        $$PWD/curve25519.cpp \
        $$PWD/env.cpp \
        $$PWD/errorhandler.cpp \
        $$PWD/externalophandler.cpp \
        $$PWD/feature.cpp \
        $$PWD/filterproxymodel.cpp \
        $$PWD/fontloader.cpp \
        $$PWD/frontend/navigator.cpp \
        $$PWD/frontend/navigationbarbutton.cpp \
        $$PWD/frontend/navigationbarmodel.cpp \
        $$PWD/frontend/navigatorreloader.cpp \
        $$PWD/glean/gleandeprecated.cpp \
        $$PWD/glean/mzglean.cpp \
        $$PWD/hacl-star/Hacl_Chacha20.c \
        $$PWD/hacl-star/Hacl_Chacha20Poly1305_32.c \
        $$PWD/hacl-star/Hacl_Curve25519_51.c \
        $$PWD/hacl-star/Hacl_Poly1305_32.c \
        $$PWD/hawkauth.cpp \
        $$PWD/hkdf.cpp \
        $$PWD/inspector/inspectorhandler.cpp \
        $$PWD/inspector/inspectoritempicker.cpp \
        $$PWD/inspector/inspectorutils.cpp \
        $$PWD/inspector/inspectorwebsocketconnection.cpp \
        $$PWD/inspector/inspectorwebsocketserver.cpp \
        $$PWD/ipaddress.cpp \
        $$PWD/itempicker.cpp \
        $$PWD/languagei18n.cpp \
        $$PWD/leakdetector.cpp \
        $$PWD/localizer.cpp \
        $$PWD/logger.cpp \
        $$PWD/loghandler.cpp \
        $$PWD/logoutobserver.cpp \
        $$PWD/models/featuremodel.cpp \
        $$PWD/models/licensemodel.cpp \
        $$PWD/networkmanager.cpp \
        $$PWD/networkrequest.cpp \
        $$PWD/qmlengineholder.cpp \
        $$PWD/qmlpath.cpp \
        $$PWD/rfc/rfc1112.cpp \
        $$PWD/rfc/rfc1918.cpp \
        $$PWD/rfc/rfc4193.cpp \
        $$PWD/rfc/rfc4291.cpp \
        $$PWD/rfc/rfc5735.cpp \
        $$PWD/settingsholder.cpp \
        $$PWD/signature.cpp \
        $$PWD/simplenetworkmanager.cpp \
        $$PWD/taskscheduler.cpp \
        $$PWD/tasks/addon/taskaddon.cpp \
        $$PWD/tasks/addonindex/taskaddonindex.cpp \
        $$PWD/tasks/authenticate/taskauthenticate.cpp \
        $$PWD/tasks/deleteaccount/taskdeleteaccount.cpp \
        $$PWD/tasks/function/taskfunction.cpp \
        $$PWD/tasks/getfeaturelist/taskgetfeaturelist.cpp \
        $$PWD/tasks/group/taskgroup.cpp \
        $$PWD/temporarydir.cpp \
        $$PWD/theme.cpp \
        $$PWD/tutorial/tutorial.cpp \
        $$PWD/tutorial/tutorialstep.cpp \
        $$PWD/tutorial/tutorialstepbefore.cpp \
        $$PWD/tutorial/tutorialstepnext.cpp \
        $$PWD/urlopener.cpp \
        $$PWD/utils.cpp \
        $$PWD/versionutils.cpp \
        $$PWD/websocket/exponentialbackoffstrategy.cpp \
        $$PWD/websocket/pushmessage.cpp \
        $$PWD/websocket/websockethandler.cpp

HEADERS += \
        $$PWD/addons/addon.h \
        $$PWD/addons/addonapi.h \
        $$PWD/addons/addonguide.h \
        $$PWD/addons/addoni18n.h \
        $$PWD/addons/addonmessage.h \
        $$PWD/addons/addonproperty.h \
        $$PWD/addons/addonpropertylist.h \
        $$PWD/addons/addontutorial.h \
        $$PWD/addons/conditionwatchers/addonconditionwatcher.h \
        $$PWD/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchergroup.h \
        $$PWD/addons/conditionwatchers/addonconditionwatcherjavascript.h \
        $$PWD/addons/conditionwatchers/addonconditionwatcherlocales.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchertime.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchertimeend.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchertimestart.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h \
        $$PWD/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h \
        $$PWD/addons/manager/addondirectory.h \
        $$PWD/addons/manager/addonindex.h \
        $$PWD/addons/manager/addonmanager.h \
        $$PWD/addons/state/addonsessionstate.h \
        $$PWD/addons/state/addonstatebase.h \
        $$PWD/addons/state/addonstate.h \
        $$PWD/app.h \
        $$PWD/authenticationlistener.h \
        $$PWD/authenticationinapp/authenticationinapp.h \
        $$PWD/authenticationinapp/authenticationinapplistener.h \
        $$PWD/authenticationinapp/authenticationinappsession.h \
        $$PWD/authenticationinapp/incrementaldecoder.h \
        $$PWD/collator.h \
        $$PWD/composer/composer.h \
        $$PWD/composer/composerblock.h \
        $$PWD/composer/composerblockbutton.h \
        $$PWD/composer/composerblocktext.h \
        $$PWD/composer/composerblocktitle.h \
        $$PWD/composer/composerblockorderedlist.h \
        $$PWD/composer/composerblockunorderedlist.h \
        $$PWD/constants.h \
        $$PWD/cryptosettings.h \
        $$PWD/curve25519.h \
        $$PWD/env.h \
        $$PWD/errorhandler.h \
        $$PWD/externalophandler.h \
        $$PWD/feature.h \
        $$PWD/filterproxymodel.h \
        $$PWD/fontloader.h \
        $$PWD/frontend/navigator.h \
        $$PWD/frontend/navigationbarbutton.h \
        $$PWD/frontend/navigationbarmodel.h \
        $$PWD/frontend/navigatorreloader.h \
        $$PWD/glean/gleandeprecated.h \
        $$PWD/glean/mzglean.h \
        $$PWD/hawkauth.h \
        $$PWD/hkdf.h \
        $$PWD/ipaddress.h \
        $$PWD/inspector/inspectorhandler.h \
        $$PWD/inspector/inspectoritempicker.h \
        $$PWD/inspector/inspectorutils.h \
        $$PWD/inspector/inspectorwebsocketconnection.h \
        $$PWD/inspector/inspectorwebsocketserver.h \
        $$PWD/itempicker.h \
        $$PWD/languagei18n.h \
        $$PWD/leakdetector.h \
        $$PWD/localizer.h \
        $$PWD/logger.h \
        $$PWD/loghandler.h \
        $$PWD/logoutobserver.h \
        $$PWD/models/featuremodel.h \
        $$PWD/models/licensemodel.h \
        $$PWD/networkmanager.h \
        $$PWD/networkrequest.h \
        $$PWD/qmlengineholder.h \
        $$PWD/qmlpath.h \
        $$PWD/rfc/rfc1112.h \
        $$PWD/rfc/rfc1918.h \
        $$PWD/rfc/rfc4193.h \
        $$PWD/rfc/rfc4291.h \
        $$PWD/rfc/rfc5735.h \
        $$PWD/settingsholder.h \
        $$PWD/signature.h \
        $$PWD/simplenetworkmanager.h \
        $$PWD/task.h \
        $$PWD/taskscheduler.h \
        $$PWD/tasks/addon/taskaddon.h \
        $$PWD/tasks/addonindex/taskaddonindex.h \
        $$PWD/tasks/authenticate/taskauthenticate.h \
        $$PWD/tasks/deleteaccount/taskdeleteaccount.h \
        $$PWD/tasks/function/taskfunction.h \
        $$PWD/tasks/getfeaturelist/taskgetfeaturelist.h \
        $$PWD/tasks/group/taskgroup.h \
        $$PWD/temporarydir.h \
        $$PWD/theme.h \
        $$PWD/tutorial/tutorial.h \
        $$PWD/tutorial/tutorialstep.h \
        $$PWD/tutorial/tutorialstepbefore.h \
        $$PWD/tutorial/tutorialstepnext.h \
        $$PWD/urlopener.h \
        $$PWD/utils.h \
        $$PWD/versionutils.h \
        $$PWD/websocket/exponentialbackoffstrategy.h \
        $$PWD/websocket/pushmessage.h \
        $$PWD/websocket/websockethandler.h

INCLUDEPATH += \
            $$PWD \
            $$PWD/addons \
            $$PWD/composer \
            $$PWD/glean \
            $$PWD/hacl-star \
            $$PWD/hacl-star/kremlin \
            $$PWD/hacl-star/kremlin/minimal

# Signal handling for unix platforms
unix {
    SOURCES += $$PWD/signalhandler.cpp
    HEADERS += $$PWD/signalhandler.h
}

ios {
    OBJECTIVE_SOURCES += \
        $$PWD/platforms/macos/macoscryptosettings.mm \
        $$PWD/platforms/ios/ioscommons.mm

    OBJECTIVE_HEADERS += \
        $$PWD/platforms/ios/ioscommons.h

    adjust {
        message(Adjust SDK enabled)
        DEFINES += MZ_ADJUST

        SOURCES += \
            $$PWD/adjust/adjustfiltering.cpp \
            $$PWD/adjust/adjusthandler.cpp \
            $$PWD/adjust/adjustproxy.cpp \
            $$PWD/adjust/adjustproxyconnection.cpp \
            $$PWD/adjust/adjustproxypackagehandler.cpp \
            $$PWD/adjust/adjusttasksubmission.cpp

        OBJECTIVE_SOURCES += $$PWD/platforms/ios/iosadjusthelper.mm

        HEADERS += \
            $$PWD/adjust/adjustfiltering.h \
            $$PWD/adjust/adjusthandler.h \
            $$PWD/adjust/adjustproxy.h \
            $$PWD/adjust/adjustproxyconnection.h \
            $$PWD/adjust/adjustproxypackagehandler.h \
            $$PWD/adjust/adjusttasksubmission.h

        OBJECTIVE_HEADERS += $$PWD/platforms/ios/iosadjusthelper.h
    }

    SWIFT_OBJC_INTERFACE_HEADER_NAME.name = "SWIFT_OBJC_INTERFACE_HEADER_NAME"
    SWIFT_OBJC_INTERFACE_HEADER_NAME.value = "Mozilla-Swift.h"
    QMAKE_MAC_XCODE_SETTINGS += SWIFT_OBJC_INTERFACE_HEADER_NAME
}

RESOURCES += shared/resources/license.qrc
RESOURCES += shared/resources/resources.qrc

CONFIG += qmltypes
QML_IMPORT_NAME = Mozilla.Shared.qmlcomponents
QML_IMPORT_MAJOR_VERSION = 1.0
