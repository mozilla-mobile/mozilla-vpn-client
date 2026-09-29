# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Build c-ares library

FetchContent_Declare(libcares SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/c-ares")

# It seems like the pipe2 syscall was added in version 27.0, but this throws
# compile warnings when we target an earlier deployment target.
if(APPLE AND (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "27.0"))
    set(HAVE_PIPE2 OFF CACHE BOOL "Force disable pipe2" FORCE)
endif()

# Set options before calling FetchContent_MakeAvailable
set(CARES_STATIC ON CACHE BOOL "Build static c-ares" FORCE)
set(CARES_SHARED OFF CACHE BOOL "Disable shared c-ares" FORCE)
set(CARES_BUILD_TESTS OFF CACHE BOOL "Disable c-ares tests" FORCE)
set(CARES_BUILD_CONTAINER_TESTS OFF CACHE BOOL "Disable c-ares container tests" FORCE)
set(CARES_BUILD_TOOLS OFF CACHE BOOL "Disable c-ares tools" FORCE)
set(CARES_INSTALL OFF CACHE BOOL "Disable c-ares global install" FORCE)

FetchContent_MakeAvailable(libcares)
