# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The tests
qt_add_executable(utest-daemonaccesscontrol testdaemonaccesscontrol.cpp testdaemonaccesscontrol.h)

get_directory_property(UTEST_ALL_TARGETS BUILDSYSTEM_TARGETS)
foreach(UTEST_TARGET ${UTEST_ALL_TARGETS})
    target_link_libraries(${UTEST_TARGET} PRIVATE mzutils-utest-harness mzdaemon)

    # Group the tests in the "Tests" folder
    set_target_properties(${UTEST_TARGET} PROPERTIES
        EXCLUDE_FROM_ALL ${BUILD_TESTS}
        FOLDER "Tests"
    )
    add_dependencies(build_tests ${UTEST_TARGET})

    # Register the test with ctest
    add_test(NAME ${UTEST_TARGET} COMMAND ${UTEST_TARGET})
    set_property(TEST ${UTEST_TARGET} PROPERTY LABELS unit)
    set_property(TEST ${UTEST_TARGET} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
endforeach()
