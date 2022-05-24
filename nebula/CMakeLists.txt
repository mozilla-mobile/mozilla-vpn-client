# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(nebula STATIC)

set_target_properties(nebula PROPERTIES FOLDER "Libs")
target_link_libraries(nebula PRIVATE Qt6::Quick)
target_include_directories(nebula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(nebula PRIVATE
    nebula.cpp
    nebula.h
    ui/components.qrc
    ui/themes.qrc
    ui/nebula_resources.qrc
    ui/compatQt6.qrc
    ui/resourcesQt6.qrc
)
