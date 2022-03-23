# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

CCACHE_BIN = $$system(which ccache)
!isEmpty(CCACHE_BIN) {
    message(Using ccache)
    load(ccache)
    QMAKE_CXXFLAGS +=-g -fdebug-prefix-map=$$PWD=.
}
