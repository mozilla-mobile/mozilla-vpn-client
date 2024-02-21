# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_utilities
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/leakdetector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/leakdetector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/temporarydir.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/temporarydir.h
        ${CMAKE_CURRENT_SOURCE_DIR}/urlopener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/urlopener.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/versionutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/versionutils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/resourceloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/resourceloader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/errorhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/errorhandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlpath.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlpath.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/androidutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/androidutils.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testtemporarydir.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testtemporarydir.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testurlopener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testurlopener.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testresourceloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testresourceloader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testqmlpath.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testqmlpath.h
    QT_DEPENDENCIES
        Core
        Gui
        Widgets
    MZ_DEPENDENCIES
        mz_logging
        mz_settings
        mz_context
        mz_telemetry
    TEST_DEPENDENCIES
        mz_translations
)
