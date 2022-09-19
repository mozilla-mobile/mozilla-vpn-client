# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

android{
    LIBS += $$PWD/../../.tmp/sentry_install/lib/libsentry.a
    LIBS += $$PWD/../../.tmp/sentry_install/lib/libunwindstack.a
    INCLUDEPATH += $$PWD/../../.tmp/sentry_install/include
    SOURCES +=  sentry/sentryadapter.cpp 
    DEFINES += SENTRY_ENABLED

    # We need custom transport on android
    DEFINES += SENTRY_TRANSPORT_ENABLED
}else{
    SOURCES +=  sentry/dummysentryadapter.cpp 
}
