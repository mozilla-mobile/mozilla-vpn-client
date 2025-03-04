# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Nothing we need to do here rn.
if(CMAKE_CROSSCOMPILING)
    return()
endif()


# This Policy will make Cmake respect SET
# over the OPTIONS of sub
cmake_policy(SET CMP0077 NEW)

include(FetchContent)

FetchContent_Declare(libcares SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/c-ares")

# Set options before calling FetchContent_MakeAvailable
set(CARES_STATIC ON CACHE BOOL "Build static c-ares" FORCE)
set(CARES_SHARED OFF CACHE BOOL "Disable shared c-ares" FORCE)
set(CARES_BUILD_TESTS OFF CACHE BOOL "Disable c-ares tests" FORCE)
set(CARES_BUILD_CONTAINER_TESTS OFF CACHE BOOL "Disable c-ares container tests" FORCE)
set(CARES_BUILD_TOOLS OFF CACHE BOOL "Disable c-ares tools" FORCE)
set(CARES_INSTALL OFF CACHE BOOL "Disable c-ares global install" FORCE)


FetchContent_MakeAvailable(libcares)
