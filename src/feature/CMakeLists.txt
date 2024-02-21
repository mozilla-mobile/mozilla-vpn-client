# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_feature
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/experimentalfeaturelist.h
        ${CMAKE_CURRENT_SOURCE_DIR}/feature.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/feature.h
        ${CMAKE_CURRENT_SOURCE_DIR}/featurelist.h
        ${CMAKE_CURRENT_SOURCE_DIR}/featurelistcallback.h
        ${CMAKE_CURRENT_SOURCE_DIR}/featuremodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/featuremodel.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testfeature.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testfeature.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testfeaturemodel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testfeaturemodel.cpp
    QT_DEPENDENCIES
        Core
        Widgets
        Qml
    MZ_DEPENDENCIES
        mz_settings
        mz_logging
)
