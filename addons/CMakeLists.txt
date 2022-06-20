# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(addons STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(addons PRIVATE Qt6::Core Qt6::Qml)
target_include_directories(addons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../scripts ABSOLUTE)
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../addons/generated/addons ABSOLUTE)
file(MAKE_DIRECTORY ${GENERATED_DIR})
target_include_directories(addons PUBLIC ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/manifest.qrc
    COMMAND python3 ${MVPN_SCRIPT_DIR}/addon/generate_all.py
)

file(GLOB_RECURSE
    manifests
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/*/manifest.json"
)

foreach(manifest ${manifests})
    if (${manifest} MATCHES "tutorial_*" OR ${manifest} MATCHES "guide_*")
        string(REGEX REPLACE "/manifest.json$" ".rcc" addon_name ${manifest})
        list(APPEND list_addons "${GENERATED_DIR}/${addon_name}")
    endif()
endforeach()

add_custom_target(generate_all_addons
    ALL
    DEPENDS ${GENERATED_DIR}/manifest.qrc
)

target_sources(addons PRIVATE
    src/addons.h
    src/addons.cpp
    ${GENERATED_DIR}/manifest.qrc
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    include(GNUInstallDirs)
    install(FILES ${list_addons} DESTINATION ${CMAKE_INSTALL_DATADIR}/mozillavpn/addons)
elseif(WIN32)
    install(FILES ${list_addons} DESTINATION addons)
endif()
