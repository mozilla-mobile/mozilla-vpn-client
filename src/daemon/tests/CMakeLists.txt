# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/tests/moc-testcases.cmake)

# The tests
qt_add_executable(utest-activate utest-activate.cpp utest-activate.h)
qt_add_executable(utest-switch utest-switch.cpp utest-switch.h)
qt_add_executable(utest-features utest-features.cpp utest-features.h)

get_directory_property(UTEST_ALL_TARGETS BUILDSYSTEM_TARGETS)
foreach(UTEST_TARGET ${UTEST_ALL_TARGETS})
    # Every test should depend on `main.cpp` as its entry point and should link to the library.
    target_sources(${UTEST_TARGET} PRIVATE main.cpp testdaemon.cpp testdaemon.h)
    target_link_libraries(${UTEST_TARGET} PRIVATE mzdaemon Qt6::Test Qt6::Core Qt6::Network)

    # Group the tests in the "Tests" folder
    set_target_properties(${UTEST_TARGET} PROPERTIES
        EXCLUDE_FROM_ALL ${BUILD_TESTS}
        FOLDER "Tests"
    )
    add_dependencies(build_tests ${UTEST_TARGET})

    # Auto-generate the test list by post-processing the moc output.
    moc_generate_testcases(${UTEST_TARGET} PARENT_CLASS TestDaemon)

    # Register the test with ctest
    add_test(NAME ${UTEST_TARGET} COMMAND ${UTEST_TARGET})
    set_property(TEST ${UTEST_TARGET} PROPERTY LABELS unit)
    set_property(TEST ${UTEST_TARGET} PROPERTY ENVIRONMENT
        LANG="en"
        LANGUAGE="en"
    )
endforeach()
