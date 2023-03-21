# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += \
        $$PWD/app.cpp \
        $$PWD/authenticationlistener.cpp \
        $$PWD/authenticationinapp/authenticationinapp.cpp \
        $$PWD/authenticationinapp/authenticationinapplistener.cpp \
        $$PWD/authenticationinapp/authenticationinappsession.cpp \
        $$PWD/authenticationinapp/incrementaldecoder.cpp \
        $$PWD/collator.cpp \
        $$PWD/constants.cpp \
        $$PWD/cryptosettings.cpp \
        $$PWD/curve25519.cpp \
        $$PWD/env.cpp \
        $$PWD/errorhandler.cpp \
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
        $$PWD/tasks/authenticate/taskauthenticate.cpp \
        $$PWD/tasks/deleteaccount/taskdeleteaccount.cpp \
        $$PWD/tasks/function/taskfunction.cpp \
        $$PWD/tasks/getfeaturelist/taskgetfeaturelist.cpp \
        $$PWD/tasks/group/taskgroup.cpp \
        $$PWD/temporarydir.cpp \
        $$PWD/theme.cpp \
        $$PWD/urlopener.cpp \
        $$PWD/utils.cpp \
        $$PWD/versionutils.cpp

HEADERS += \
        $$PWD/app.h \
        $$PWD/authenticationlistener.h \
        $$PWD/authenticationinapp/authenticationinapp.h \
        $$PWD/authenticationinapp/authenticationinapplistener.h \
        $$PWD/authenticationinapp/authenticationinappsession.h \
        $$PWD/authenticationinapp/incrementaldecoder.h \
        $$PWD/collator.h \
        $$PWD/constants.h \
        $$PWD/cryptosettings.h \
        $$PWD/curve25519.h \
        $$PWD/env.h \
        $$PWD/errorhandler.h \
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
        $$PWD/tasks/authenticate/taskauthenticate.h \
        $$PWD/tasks/deleteaccount/taskdeleteaccount.h \
        $$PWD/tasks/function/taskfunction.h \
        $$PWD/tasks/getfeaturelist/taskgetfeaturelist.h \
        $$PWD/tasks/group/taskgroup.h \
        $$PWD/temporarydir.h \
        $$PWD/theme.h \
        $$PWD/urlopener.h \
        $$PWD/utils.h \
        $$PWD/versionutils.h

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
}

RESOURCES += shared/resources/license.qrc
