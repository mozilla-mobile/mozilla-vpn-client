# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

linux:!android|macos|win* {
    win* {
      LIBNAME=signature.lib
    } else {
      LIBNAME=libsignature.a
    }

    signatureCrate.input = SIGNATURE_CRATE
    signatureCrate.output = ${QMAKE_FILE_IN}/target/release/$$LIBNAME
    signatureCrate.commands = @echo Building signature rust create staticlib \
        && cd ${QMAKE_FILE_IN} \
        && cargo build --release
    signatureCrate.clean = ${QMAKE_FILE_OUT}
    signatureCrate.CONFIG = target_predeps
    signatureCrate.depends += ${QMAKE_FILE_IN}/Cargo.toml ${QMAKE_FILE_IN}/src/lib.rs
    signatureCrate.variable_out = LIBS

    QMAKE_EXTRA_COMPILERS += signatureCrate
    SIGNATURE_CRATE = $$PWD/../../signature
    DEFINES += MVPN_SIGNATURE
}
