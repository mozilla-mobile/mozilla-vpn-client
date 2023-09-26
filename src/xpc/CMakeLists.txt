#This Source Code Form is subject to the terms of the Mozilla Public
#License, v.2.0. If a copy of the MPL was not distributed with this
#file, You can obtain one at http:  // mozilla.org/MPL/2.0/.

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    return () 
endif()

target_sources(shared-sources PUBLIC 
    xpcbase.h
    xpcservice.h 
    xpcservice.cpp 
    xpcclient.h 
    xpcclient.cpp
)
add_subdirectory(examples)
