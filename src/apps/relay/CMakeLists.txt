# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(MZ_PLATFORM_NAME "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(MZ_PLATFORM_NAME "windows")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MZ_PLATFORM_NAME "macos")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    set(MZ_PLATFORM_NAME "android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(MZ_PLATFORM_NAME "ios")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(MZ_PLATFORM_NAME "wasm")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/shared)
include_directories(${MZ_SOURCE_DIR}/shared/glean)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)
include_directories(${MZ_SOURCE_DIR}/apps/relay)

qt_add_executable(relay EXCLUDE_FROM_ALL MANUAL_FINALIZATION)
add_dependencies(relayapp relay)
target_compile_definitions(relay PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")

target_link_libraries(relay PRIVATE
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(relay PRIVATE
        Qt6::NetworkAuth
    )
endif()

target_link_libraries(relay PRIVATE
    glean
    lottie
    nebula
    qtglean
    shared-sources
    translations_relay
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(relay PRIVATE Qt6::DBus)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(relay PRIVATE PkgConfig::libsecret)
endif()

target_compile_definitions(relay PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")

target_sources(relay PRIVATE
    appconstants.cpp
    appconstants.h
    appfeaturelistcallback.h
    appfeaturelist.h
    appsettingslist.h
    main.cpp
    relayapp.cpp
    relayapp.h
)

# Resources
target_sources(relay PRIVATE
  ui/ui.qrc
)

qt_finalize_target(relay)
