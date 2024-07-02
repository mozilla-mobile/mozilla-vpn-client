# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


mz_add_test_target(testSocks5client
    SOURCES
        testsocks5client.cpp
        testsocks5client.h
    DEPENDENCIES
        libSocks5proxy
)

mz_add_test_target(testSocks5
    SOURCES
        testsocks5.cpp
        testsocks5.h
    DEPENDENCIES
        libSocks5proxy
)
