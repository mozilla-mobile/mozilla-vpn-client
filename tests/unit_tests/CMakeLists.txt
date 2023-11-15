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
include_directories(${MZ_SOURCE_DIR}/hacl-star)
include_directories(${MZ_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/hacl-star/kremlin/minimal)
include_directories(${CMAKE_SOURCE_DIR}/tests/unit_tests)

qt_add_executable(app_unit_tests EXCLUDE_FROM_ALL MANUAL_FINALIZATION)
add_dependencies(build_tests app_unit_tests)
target_compile_definitions(app_unit_tests PRIVATE UNIT_TEST "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")
target_compile_definitions(app_unit_tests PRIVATE MZ_ADJUST)

target_link_libraries(app_unit_tests PRIVATE
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(app_unit_tests PRIVATE
        Qt6::NetworkAuth
    )
endif()

include(${CMAKE_SOURCE_DIR}/scripts/cmake/generate_translations_target.cmake)
generate_translations_target(translations-unit-tests ${CMAKE_SOURCE_DIR}/tests/unit_tests/translations)

target_link_libraries(app_unit_tests PRIVATE
    qtglean
    shared-sources
    translations-unit-tests
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(app_unit_tests PRIVATE Qt6::DBus)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(app_unit_tests PRIVATE PkgConfig::libsecret)
endif()

target_compile_definitions(app_unit_tests PRIVATE MZ_DEBUG)
target_compile_definitions(app_unit_tests PRIVATE MZ_DUMMY)

# Unit test source files
target_sources(app_unit_tests PRIVATE
    helper.h
    main.cpp
    mocmozillavpn.cpp
    testaddon.cpp
    testaddon.h
    testaddonapi.cpp
    testaddonapi.h
    testaddonindex.cpp
    testaddonindex.h
    testaddonsessionstate.cpp
    testaddonsessionstate.h
    testaddonstatebase.cpp
    testaddonstatebase.h
    testadjust.cpp
    testadjust.h
    testcheckedint.cpp
    testcheckedint.h
    testcomposer.cpp
    testcomposer.h
    testenv.cpp
    testenv.h
    testfeature.cpp
    testfeature.h
    testfeaturemodel.cpp
    testfeaturemodel.h
    testipaddress.cpp
    testipaddress.h
    testlanguagei18n.cpp
    testlanguagei18n.h
    testlicense.cpp
    testlicense.h
    testlocalizer.cpp
    testlocalizer.h
    testlogger.cpp
    testlogger.h
    testnetworkmanager.cpp
    testnetworkmanager.h
    testqmlpath.cpp
    testqmlpath.h
    testresourceloader.cpp
    testresourceloader.h
    testtasksentry.cpp
    testtasksentry.h
    testsettings.cpp
    testsettings.h
    testtasks.cpp
    testtasks.h
    testtemporarydir.cpp
    testtemporarydir.h
    testthemes.cpp
    testthemes.h
    testurlopener.cpp
    testurlopener.h
    ${MZ_SOURCE_DIR}/mozillavpn.h
    ${MZ_SOURCE_DIR}/sentry/sentryadapter.h
    ${MZ_SOURCE_DIR}/tasks/sentry/tasksentry.cpp
    ${MZ_SOURCE_DIR}/tasks/sentry/tasksentry.h
)

if(NOT BUILD_ADJUST_SDK_TOKEN)
    target_sources(app_unit_tests PRIVATE
        ${MZ_SOURCE_DIR}/adjust/adjustfiltering.cpp
        ${MZ_SOURCE_DIR}/adjust/adjustfiltering.h
        ${MZ_SOURCE_DIR}/adjust/adjustproxypackagehandler.cpp
        ${MZ_SOURCE_DIR}/adjust/adjustproxypackagehandler.h
    )
endif()

## Add the tests to be run, one for each test class.
get_target_property(UTEST_SOURCES app_unit_tests SOURCES)
list(FILTER UTEST_SOURCES INCLUDE REGEX "test.*.h$")
foreach(filename ${UTEST_SOURCES})
    execute_process(
        OUTPUT_VARIABLE UTEST_CLASS_LIST
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/tests/list_test_classes.py -p TestHelper ${filename}
    )

    foreach(UTEST_CLASS ${UTEST_CLASS_LIST})
        add_test(NAME ${UTEST_CLASS} COMMAND app_unit_tests ${UTEST_CLASS})
        set_property(TEST ${UTEST_CLASS} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
    endforeach()
endforeach()

# Unit test mock resources
target_sources(app_unit_tests PRIVATE
    addons/addons.qrc
    guides/guides.qrc
    qml/qml.qrc
    themes/themes.qrc
    resourceloader/resourceloader.qrc
)

qt_finalize_target(app_unit_tests)
