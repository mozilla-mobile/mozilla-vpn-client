# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The tests
qt_add_executable(utest-chacha20poly testchacha20poly.cpp testchacha20poly.h)
qt_add_executable(utest-commandlineparser testcommandlineparser.cpp testcommandlineparser.h)
qt_add_executable(utest-curve25519 testcurve25519.cpp testcurve25519.h)
qt_add_executable(utest-hkdf testhkdf.cpp testhkdf.h)
qt_add_executable(utest-ipaddress testipaddress.cpp testipaddress.h)
qt_add_executable(utest-logger testlogger.cpp testlogger.h)
qt_add_executable(utest-tasks testtasks.cpp testtasks.h)
qt_add_executable(utest-servermodels testservermodels.cpp testservermodels.h)

get_directory_property(UTEST_ALL_TARGETS BUILDSYSTEM_TARGETS)
foreach(UTEST_TARGET ${UTEST_ALL_TARGETS})
    # Every test should depend on `main.cpp` as its entry point and should link to the library.
    target_sources(${UTEST_TARGET} PRIVATE main.cpp testhelper.h)
    target_link_libraries(${UTEST_TARGET} PRIVATE mzutils Qt6::Test Qt6::Core Qt6::Network)

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
