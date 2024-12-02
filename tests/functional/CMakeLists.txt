# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Add ctests for each of the functional test scripts
file(GLOB FTEST_SOURCES
    LIST_DIRECTORIES FALSE
    RELATIVE ${CMAKE_SOURCE_DIR}
    "test*.js"
)

# We must build the test addons for functional testing.
add_subdirectory(addons)

foreach(FILENAME ${FTEST_SOURCES})
    get_filename_component(FTEST_NAME ${FILENAME} NAME_WLE)
    add_test(NAME ${FTEST_NAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND npm run functionalTest -- ${FILENAME})
    
    set_tests_properties(${FTEST_NAME} PROPERTIES
        RUN_SERIAL TRUE
        LABELS functional
    )
    set_property(TEST ${FTEST_NAME} PROPERTY ENVIRONMENT
        MVPN_BIN=$<TARGET_FILE:mozillavpn>
        MVPN_ADDONS_PATH=${CMAKE_CURRENT_BINARY_DIR}/addons
    )
    set_property(TEST ${FTEST_NAME} PROPERTY REQUIRED_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/addons/prod/manifest.json
    )
endforeach()

file(GLOB ADDON_TARGETS
    LIST_DIRECTORIES TRUE
    RELATIVE ${CMAKE_SOURCE_DIR}/addons
    "[0-9][0-9]_*")

foreach(SCENARIO ${ADDON_TARGETS})
    set_property(TEST testAddons APPEND PROPERTY REQUIRED_FILES
        ${CMAKE_BINARY_DIR}/addons/${SCENARIO}/manifest.json
    )
endforeach()
