# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/shared)
include_directories(${MZ_SOURCE_DIR}/shared/glean)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)
include_directories(${MZ_SOURCE_DIR}/apps/unit_tests)
include_directories(${MZ_SOURCE_DIR}/apps/vpn)

qt_add_executable(app_unit_tests EXCLUDE_FROM_ALL)
add_dependencies(build_tests app_unit_tests)
target_compile_definitions(app_unit_tests PRIVATE UNIT_TEST)

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

target_link_libraries(app_unit_tests PRIVATE
    glean
    vpnglean
    shared-sources
    translations
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
    ${MZ_SOURCE_DIR}/apps/vpn/glean/glean.h
    ${MZ_SOURCE_DIR}/apps/vpn/glean/glean.cpp
    languages/languages.qrc
    appconstants.h
    appfeaturelistcallback.h
    appfeaturelist.h
    appsettingslist.h
    main.cpp
    helper.h
    testcheckedint.h
    testcheckedint.cpp
    testfeature.cpp
    testfeature.h
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
    testsettings.cpp
    testsettings.h
    testtemporarydir.cpp
    testtemporarydir.h
    testthemes.cpp
    testthemes.h
    testurlopener.cpp
    testurlopener.h
)

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
    qml/qml.qrc
    themes/themes.qrc
)

qt_finalize_target(app_unit_tests)
