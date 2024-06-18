# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


mz_add_test_target(testSocks5client
    SOURCES
        testSocks5client.cpp
        testSocks5client.h
    DEPENDENCIES
        libSocks5proxy
)
