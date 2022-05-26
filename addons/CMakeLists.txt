# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(addons STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(addons PRIVATE Qt6::Core Qt6::Qml)

get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../scripts ABSOLUTE)
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)
file(MAKE_DIRECTORY ${GENERATED_DIR})
target_include_directories(addons PUBLIC ${GENERATED_DIR})

file(GLOB_RECURSE
    manifests
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/*/manifest.json"
)

foreach(manifest ${manifests})
    string(REGEX REPLACE "/manifest.json$" ".rcc" addon_name ${manifest})
    if (${manifest} MATCHES "tutorial_*" OR ${manifest} MATCHES "guide_*")
        add_custom_command(
            OUTPUT ${GENERATED_DIR}/${addon_name}
            DEPENDS ${manifest}
            COMMAND python3 ${MVPN_SCRIPT_DIR}/addon/build.py
               ${CMAKE_CURRENT_SOURCE_DIR}/${manifest} ${GENERATED_DIR}
        )
        list(APPEND list_addons "${GENERATED_DIR}/${addon_name}")
    endif()
endforeach()

add_custom_target(generate_all_addons
    ALL
    DEPENDS ${list_addons}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    include(GNUInstallDirs)
    install(FILES ${list_addons} DESTINATION ${CMAKE_INSTALL_DATADIR}/mozillavpn/addons)
elseif(WIN32)
    install(FILES ${list_addons} DESTINATION addons)
endif()
