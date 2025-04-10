# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(nativemessaging_tests)
set_target_properties(nativemessaging_tests PROPERTIES
    EXCLUDE_FROM_ALL ${BUILD_TESTS}
    FOLDER "Tests"
)
add_dependencies(build_tests nativemessaging_tests)

target_link_libraries(nativemessaging_tests PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

# Native messaging test sources
target_sources(nativemessaging_tests PRIVATE
    main.cpp
    helper.cpp
    helper.h
    helperserver.cpp
    helperserver.h
    testbridge.cpp
    testbridge.h
    testnoop.cpp
    testnoop.h
    test_illegal_invoke.cpp
    test_illegal_invoke.h
)

add_dependencies(nativemessaging_tests mozillavpnnp)
add_test(NAME nativemessaging_tests COMMAND nativemessaging_tests $<TARGET_FILE:mozillavpnnp>)
set_property(TEST nativemessaging_tests PROPERTY LABELS nativemessaging)
