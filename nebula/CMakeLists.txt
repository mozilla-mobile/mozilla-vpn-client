# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(nebula STATIC)

add_subdirectory(ui/components)
add_subdirectory(ui/compat)
add_subdirectory(ui/themes)
add_subdirectory(ui/resources)
add_subdirectory(ui/utils)


set_target_properties(nebula PROPERTIES FOLDER "Libs")
target_link_libraries(nebula PRIVATE 
    Qt6::Quick
    components
    componentsplugin
    compat
    compatplugin
    themes
    themesplugin
    resources
    resourcesplugin
    utils
    utilsplugin
)

target_include_directories(nebula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(nebula PRIVATE
    nebula.cpp
    nebula.h
)