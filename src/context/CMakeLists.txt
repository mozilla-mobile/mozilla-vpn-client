# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Generate version header file
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

mz_add_new_module(
    TARGET_NAME mz_context
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/env.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/env.h
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h
    QT_DEPENDENCIES
        Core
        Network
        Widgets
        Quick
    MZ_DEPENDENCIES
        mz_settings
)
