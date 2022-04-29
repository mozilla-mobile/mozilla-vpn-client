# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_compile_options(/MP /Zc:preprocessor)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(/Z7 /ZI /DEBUG)
    add_link_options(/DEBUG)
endif()
