# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Nothing we need to do here rn.
if((NOT CMAKE_SYSTEM_NAME STREQUAL "Windows") AND
   (NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin") AND
   (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    return()
endif()

# This Policy will make Cmake respect SET
# over the OPTIONS of sub
cmake_policy(SET CMP0077 NEW)

include(FetchContent)

# Build c-ares
include(${CMAKE_CURRENT_LIST_DIR}/c-ares.cmake)

# Build sentry
include(${CMAKE_CURRENT_LIST_DIR}/sentry.cmake)
