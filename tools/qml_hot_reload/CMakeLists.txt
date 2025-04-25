# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/. */

if(CMAKE_CROSSCOMPILING)
    return()
endif()

add_executable(hotreloader
    main.cpp
    filewatcherclient.h
    filewatcherclient.cpp
)
set_target_properties(hotreloader PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        FOLDER "Tools"
)

target_compile_definitions(hotreloader PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(hotreloader PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)
