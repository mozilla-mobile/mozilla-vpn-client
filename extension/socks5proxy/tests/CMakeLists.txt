# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_test_target(testSocks5connection
    SOURCES
        testsocks5connection.cpp
        testsocks5connection.h
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

# For testing named pipe support, add a little wrapper tool to run curl through
# a named pipe. This should let us debug while we try to figure out how to get
# Firefox to use them.
if(WIN32)
    qt_add_executable(curlpipe curlpipe.cpp)
    target_link_libraries(curlpipe PUBLIC 
        Qt6::Core 
        Qt6::Network
    )
endif()
