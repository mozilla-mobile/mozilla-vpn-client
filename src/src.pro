# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This is the entry point of the MozillaVPN qmake project.
#
# Because this is a complex project, with dependencies, extra compilers,
# modules, etc, each logical block is included as a separate PRI file.

TEMPLATE  = app

include($$PWD/qmake/balrog.pri)
include($$PWD/qmake/ccache.pri)
include($$PWD/qmake/debug.pri)
include($$PWD/qmake/glean.pri)
include($$PWD/qmake/golang.pri)
include($$PWD/qmake/includes_and_defines.pri)
include($$PWD/qmake/qt.pri)
include($$PWD/qmake/translations.pri)
include($$PWD/qmake/webextension.pri)
include($$PWD/../nebula/nebula.pri)
include($$PWD/../lottie/lottie.pri)
include($$PWD/crashreporter/crashreporter.pri)

# Cross-platform entries go in here:
include($$PWD/qmake/sources.pri)

# Platform-specific entries:
DUMMY {
   include($$PWD/qmake/platforms/dummy.pri)
} else:linux:!android {
   include($$PWD/qmake/platforms/linux.pri)
} else:android {
   include($$PWD/qmake/platforms/android.pri)
} else:macos {
   include($$PWD/qmake/platforms/macos.pri)
} else:ios {
   include($$PWD/qmake/platforms/ios.pri)
} else:win* {
   include($$PWD/qmake/platforms/windows.pri)
} else:wasm {
   include($$PWD/qmake/platforms/wasm.pri)
} else {
   error(Unsupported platform)
}
