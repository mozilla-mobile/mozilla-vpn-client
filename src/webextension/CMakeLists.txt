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
        tests/testbaseadapter.cpp
        tests/testbaseadapter.h
        tests/testconnection.cpp
        tests/testconnection.h
        tests/testserver.cpp
        tests/testserver.h
    QT_DEPENDENCIES
        Core
        Network
)
