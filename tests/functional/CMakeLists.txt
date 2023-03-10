# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file(GLOB FUNCTIONAL_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test*.js")
foreach(FUNCTIONAL_TEST_SOURCE ${FUNCTIONAL_TEST_SOURCES})
    get_filename_component(FUNCTIONAL_TEST_BASENAME ${FUNCTIONAL_TEST_SOURCE} NAME_WE)
    get_filename_component(FUNCTIONAL_TEST_ABSPATH ${FUNCTIONAL_TEST_SOURCE} ABSOLUTE)
    add_test(NAME ${FUNCTIONAL_TEST_BASENAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env
                    MVPN_BIN=$<TARGET_FILE:dummyvpn>
                npm run functionalTest ${FUNCTIONAL_TEST_ABSPATH})
endforeach()
