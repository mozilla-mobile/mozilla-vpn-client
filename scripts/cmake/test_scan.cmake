# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# register_tests <target>
# Searches all headers files in <target> that start with "test"
# Finds all classes that extend TestHelper and will register a new C-Test 
# for that Binary+TestClass

function(register_tests test_target)

    ## Add the tests to be run, one for each test class.
    get_target_property(UTEST_SOURCES ${test_target} SOURCES)

    list(FILTER UTEST_SOURCES INCLUDE REGEX "test.*.h$")

    foreach(filename ${UTEST_SOURCES})
        execute_process(
            OUTPUT_VARIABLE UTEST_CLASS_LIST
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/tests/list_test_classes.py -p TestHelper ${filename}
        )

        foreach(UTEST_CLASS ${UTEST_CLASS_LIST})
            add_test(NAME ${UTEST_CLASS} COMMAND ${test_target} ${UTEST_CLASS})
            set_property(TEST ${UTEST_CLASS} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
        endforeach()
    endforeach()

endfunction(register_tests)