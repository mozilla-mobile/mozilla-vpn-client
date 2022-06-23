# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SOURCES += $$PWD/lib/lottie.cpp \
           $$PWD/lib/lottieprivate.cpp \
           $$PWD/lib/lottieprivatedocument.cpp \
           $$PWD/lib/lottieprivatenavigator.cpp \
           $$PWD/lib/lottieprivatewindow.cpp

HEADERS += $$PWD/lib/lottie.h \
           $$PWD/lib/lottieprivate.h \
           $$PWD/lib/lottieprivatedocument.h \
           $$PWD/lib/lottieprivatenavigator.h \
           $$PWD/lib/lottieprivatewindow.h \
           $$PWD/lib/lottiestatus.h

# wrap lottie in a single file before using it as resource.
LOTTIEWRAP = $$cat($$PWD/lib/lottie/lottie_wrap.js.template, blob)
LOTTIEJS = $$cat($$PWD/lib/lottie/lottie.min.js, blob)
LOTTIERC = $$replace(LOTTIEWRAP, @LOTTIEJS@, $$LOTTIEJS)
write_file($$PWD/lib/lottie/lottie.mjs, LOTTIERC)

RESOURCES += $$PWD/lib/lottie.qrc
