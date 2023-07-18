# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


qt_add_executable(sentry_tests EXCLUDE_FROM_ALL)
add_dependencies(build_tests sentry_tests)
add_dependencies(app_unit_tests sentry_tests)


target_link_libraries(sentry_tests PRIVATE
    qtglean
    shared-sources
    translations_unit_tests
    app_unit_test_core
)


target_sources( sentry_tests PRIVATE
    testsentryadapter.cpp
    testsentryadapter.h
)

register_tests(sentry_tests)