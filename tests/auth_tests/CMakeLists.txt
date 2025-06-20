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
include_directories(${MZ_SOURCE_DIR}/glean)
include_directories(${MZ_SOURCE_DIR}/ui/composer)
include_directories(${CMAKE_SOURCE_DIR}/tests/auth_tests)

qt_add_executable(app_auth_tests MANUAL_FINALIZATION)
set_target_properties(app_auth_tests PROPERTIES
    EXCLUDE_FROM_ALL ${BUILD_TESTS}
    FOLDER "Tests"
)
add_dependencies(build_tests app_auth_tests)
target_compile_definitions(app_auth_tests PRIVATE UNIT_TEST "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")

target_link_libraries(app_auth_tests PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Svg
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(app_auth_tests PRIVATE
        Qt6::NetworkAuth
    )
endif()

target_link_libraries(app_auth_tests PRIVATE
    qtglean
    mzutils
    shared-sources
    translations
)

target_compile_definitions(app_auth_tests PRIVATE MZ_DEBUG)

# Unit test source files
target_sources(app_auth_tests PRIVATE
    main.cpp
    mocmozillavpn.cpp
    ${MZ_SOURCE_DIR}/mozillavpn.h
    testemailvalidation.cpp
    testemailvalidation.h
    testpasswordvalidation.cpp
    testpasswordvalidation.h
    testsignupandin.cpp
    testsignupandin.h
    ${MZ_SOURCE_DIR}/ui/composer/composer.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composer.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblock.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblock.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblockbutton.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblockbutton.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblockorderedlist.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblockorderedlist.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblocktext.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblocktext.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblocktitle.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblocktitle.h
    ${MZ_SOURCE_DIR}/ui/composer/composerblockunorderedlist.cpp
    ${MZ_SOURCE_DIR}/ui/composer/composerblockunorderedlist.h
)

## Add the tests to be run, one for each test class.
get_target_property(AUTH_TEST_SOURCES app_auth_tests SOURCES)
list(FILTER AUTH_TEST_SOURCES INCLUDE REGEX "test.*.h$")
foreach(filename ${AUTH_TEST_SOURCES})
    execute_process(
        OUTPUT_VARIABLE AUTH_TEST_CLASS_LIST
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/tests/list_test_classes.py -p QObject ${filename}
    )

    foreach(AUTH_TEST_CLASS ${AUTH_TEST_CLASS_LIST})
        add_test(NAME ${AUTH_TEST_CLASS} COMMAND app_auth_tests ${AUTH_TEST_CLASS})
        set_property(TEST ${AUTH_TEST_CLASS} PROPERTY LABELS auth)
        set_property(TEST ${AUTH_TEST_CLASS} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
    endforeach()
endforeach()

qt_finalize_target(app_auth_tests)
