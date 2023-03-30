# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This is the entry point of the MozillaVPN qmake project.
#
# Because this is a complex project, with dependencies, extra compilers,
# modules, etc, each logical block is included as a separate PRI file.

TEMPLATE = app

include($$PWD/apps/vpn/qmake/debug.pri)
include($$PWD/apps/vpn/qmake/includes_and_defines.pri)
include($$PWD/apps/vpn/qmake/qt.pri)
include($$PWD/../glean/glean.pri)
include($$PWD/../nebula/nebula.pri)
include($$PWD/../lottie/lottie.pri)
include($$PWD/../translations/translations.pri)
include($$PWD/../qtglean/qtglean.pri)

unix {
   include($$PWD/apps/vpn/qmake/ccache.pri)
   include($$PWD/apps/vpn/qmake/golang.pri)
}

# Cross-platform entries go in here:
include($$PWD/shared/sources.pri)
include($$PWD/apps/vpn/qmake/sources.pri)

# Platform-specific entries:
DUMMY {
   include($$PWD/apps/vpn/qmake/platforms/dummy.pri)
} else:linux:!android {
   error(qmake is not supported for Linux. Please use cmake.)
} else:android {
   error(qmake is not supported for Android. Please use cmake.)
} else:ios {
   include($$PWD/apps/vpn/qmake/platforms/ios.pri)
} else:linux {
   error(qmake is not supported for Linux. Please use cmake.)
} else:macos {
   error(qmake is not supported for MacOS. Please use cmake.)
} else:win* {
   error(qmake is not supported for Windows. Please use cmake.)
} else:wasm {
   error(qmake is not supported for Wasm. Please use cmake.)
} else {
   error(Unsupported platform)
}
