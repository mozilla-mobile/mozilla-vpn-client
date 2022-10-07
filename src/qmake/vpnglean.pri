# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

win* {
  LIBNAME=vpnglean.lib
} else {
  LIBNAME=libvpnglean.a
}

vpngleanCrate.input = VPNGLEAN_CRATE
vpngleanCrate.output = ${QMAKE_FILE_IN}/target/universal/release/$$LIBNAME
vpngleanCrate.commands = @echo Building vpnglean rust create staticlib \
    && cd ${QMAKE_FILE_IN}
    && cargo lipo --release
vpngleanCrate.clean = ${QMAKE_FILE_OUT}
vpngleanCrate.CONFIG = target_predeps
vpngleanCrate.depends += ${QMAKE_FILE_IN}/Cargo.toml ${QMAKE_FILE_IN}/src/lib.rs
vpngleanCrate.variable_out = LIBS

QMAKE_EXTRA_COMPILERS += vpngleanCrate
VPNGLEAN_CRATE = $$PWD/../../vpnglean
DEFINES += MVPN_VPNGLEAN
