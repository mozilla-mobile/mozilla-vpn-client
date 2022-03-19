# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

linux:!android|macos|win* {
    message(Enabling the webextension support)

    DEFINES += MVPN_WEBEXTENSION

    SOURCES += \
            $$PWD/../server/serverconnection.cpp \
            $$PWD/../server/serverhandler.cpp
    HEADERS += \
            $$PWD/../server/serverconnection.h \
            $$PWD/../server/serverhandler.h

    # This cannot be an EXTRA_TARGET because we want to 'clean' the cargo build
    # too when `make clean` runs. and QMAKE_CLEAN is not enough (it does not
    # run scripts such as `cargo clean`)

    cargoBridge.input = CARGO_CRATE
    cargoBridge.output = ${QMAKE_FILE_IN}/target/release/mozillavpnnp
    cargoBridge.commands = python3 $$PWD/../../scripts/qmake/webextension.py build
    cargoBridge.clean_commands = python3 $$PWD/../../scripts/qmake/webextension.py clean
    cargoBridge.CONFIG = target_predeps no_link

    QMAKE_EXTRA_COMPILERS += cargoBridge
    CARGO_CRATE = ../extension/bridge
}
