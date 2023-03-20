# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Build the addons for testing.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.20)
    include(${CMAKE_SOURCE_DIR}/scripts/cmake/addons.cmake)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/addons)

    add_custom_target(test_01_empty_manifest
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/addons/01_empty_manifest/
                    ${CMAKE_CURRENT_BINARY_DIR}/addons/01_empty_manifest/
    )
    add_dependencies(addons test_01_empty_manifest)

    add_custom_target(test_02_broken_manifest
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/addons/02_broken_manifest/
                    ${CMAKE_CURRENT_BINARY_DIR}/addons/02_broken_manifest/
    )
    add_dependencies(addons test_02_broken_manifest)

    add_addon_target(test_03_single_addon
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/addons/03_single_addon/generated/addons/
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/addons/03_single_addon/
    )
    add_dependencies(addons test_03_single_addon)

    add_addon_target(test_04_tutorials_basic
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/addons/04_tutorials_basic/generated/addons/
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/addons/04_tutorials_basic/
    )
    add_dependencies(addons test_04_tutorials_basic)

    add_addon_target(test_05_settings_rollback
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/addons/05_settings_rollback/generated/addons/
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/addons/05_settings_rollback/
    )
    add_dependencies(addons test_05_settings_rollback)

    add_addon_target(test_06_translation_threshold
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/addons/06_translation_threshold/generated/addons/
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/addons/06_translation_threshold/
    )
    add_dependencies(addons test_06_translation_threshold)
endif()

## Don't add the functional tests unless specifically enabled, since they significantly
## increase the complexity and runtime of the test suite.
if(NOT BUILD_FUNCTIONAL_TESTS)
    return()
endif()

# Generate the dummy client and addons when building tests
add_dependencies(build_tests dummyvpn)
add_dependencies(build_tests addons)

file(GLOB FUNCTIONAL_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test*.js")
foreach(FUNCTIONAL_TEST_SOURCE ${FUNCTIONAL_TEST_SOURCES})
    get_filename_component(FUNCTIONAL_TEST_BASENAME ${FUNCTIONAL_TEST_SOURCE} NAME_WE)
    get_filename_component(FUNCTIONAL_TEST_ABSPATH ${FUNCTIONAL_TEST_SOURCE} ABSOLUTE)
    add_test(NAME ${FUNCTIONAL_TEST_BASENAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env
                    MVPN_BIN=$<TARGET_FILE:dummyvpn>
                    MVPN_PROD_ADDONS_PATH=${CMAKE_BINARY_DIR}/addons
                    MVPN_TEST_ADDONS_PATH=${CMAKE_CURRENT_BINARY_DIR}/addons
                npm run functionalTest ${FUNCTIONAL_TEST_ABSPATH})
endforeach()
