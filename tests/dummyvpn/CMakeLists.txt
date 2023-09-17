# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(dummyvpn MANUAL_FINALIZATION)
include(ExternalProject)
target_link_libraries(dummyvpn PRIVATE
    Qt6::Quick
    Qt6::qmlplugin
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" 
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(dummyvpn PRIVATE
        Qt6::NetworkAuth
    )
endif()

target_link_libraries(dummyvpn PRIVATE
    shared-sources
    mozillavpn-sources
    lottie
    nebula
    translations
    qtglean
)

target_sources(dummyvpn PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/dummy/dummycontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/dummy/dummycontroller.h
)

# On Windows, Qt is linked statically and requires the Qt built-in plugins.
# qmlimportscanner scans the source directory for qml resources in qrc files,
# extracts their import statements, and generates a list of libraries that
# supply these built-in plugins.
# dummyvpn/qml/qt6winhack.qml contains the necessary plugin imports.
if(WIN32)
    target_sources(dummyvpn PRIVATE  ${CMAKE_SOURCE_DIR}/tests/dummyvpn/qml/qt6winhack.qrc)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(dummyvpn PRIVATE Qt6::DBus)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(dummyvpn PRIVATE PkgConfig::libsecret)
endif()

target_compile_definitions(dummyvpn PRIVATE MZ_DEBUG)

# MZ_DUMMY is a separate platform for dummyvpn.
# Use #ifdef _WIN32 instead of MZ_WINDOWS in dummyvpn for Windows-specific code.
target_compile_definitions(dummyvpn PRIVATE MZ_DUMMY)

# We should not build into the source dir
# but that is how we are currently doing things, 
# at least we no longer invoke python scripts to 
# invoke cmake. 
#  ¯\_(ツ)_/¯
ExternalProject_Add(functional_test_addons
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests/functional/addons
    BINARY_DIR ${CMAKE_SOURCE_DIR}/tests/functional/addons/generated
    CMAKE_CACHE_ARGS -DQt6_DIR:PATH=${Qt6_DIR} -DQt6QmlTools_DIR:PATH=${Qt6QmlTools_DIR} -DQt6CoreTools_DIR:PATH=${Qt6CoreTools_DIR}
    INSTALL_COMMAND ""
)
add_dependencies(dummyvpn functional_test_addons)

qt6_add_qml_module(dummyvpn
  URI Mozilla.Shared.qmlcomponents
  VERSION 1.0
)

qt_finalize_target(dummyvpn)
