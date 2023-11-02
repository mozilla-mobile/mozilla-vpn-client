# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Windows Qt6 UI workaround resources
if (WIN32)
    target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt6winhack.qrc)
endif()

# We should not build into the source dir
# but that is how we are currently doing things,
# at least we no longer invoke python scripts to
# invoke cmake.
#  ¯\_(ツ)_/¯
ExternalProject_Add(functional_test_addons
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests/functional/addons
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/addons
    CMAKE_CACHE_ARGS -DCMAKE_PREFIX_PATH:PATH=${QT6_PREFIX_PATH}
    INSTALL_COMMAND ""
)
add_dependencies(mozillavpn functional_test_addons)
