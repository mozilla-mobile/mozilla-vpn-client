# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_webextension
    SOURCES
        baseadapter.h
        baseadapter.cpp
        connection.cpp
        connection.h
        server.cpp
        server.h
    TEST_SOURCES
        tests/testBaseAdapter.cpp
        tests/testBaseAdapter.h
        tests/testConnection.cpp
        tests/testConnection.h
        tests/testServer.cpp
        tests/testServer.h
    QT_DEPENDENCIES
        Core
        Network
)

target_sources(mozillavpn-sources INTERFACE
    vpnadapter.h
    vpnadapter.cpp
)
target_compile_definitions(mozillavpn-sources INTERFACE MVPN_WEBEXTENSION)
