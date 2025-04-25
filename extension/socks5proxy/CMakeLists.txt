# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# No need to compile this for mobile/web
if(CMAKE_CROSSCOMPILING)
    return()
endif()

add_subdirectory(src)
add_subdirectory(bin)
add_subdirectory(tests)

target_compile_definitions(shared-sources INTERFACE MZ_PROXY_ENABLED)



