# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" ) 
    return()
endif()


add_library(xpc INTERFACE
    xpcservice.h
    xpcservice.cpp
    XPCCLient.h
    XPCCLient.cpp
    
)

