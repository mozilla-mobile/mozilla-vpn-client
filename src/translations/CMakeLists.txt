# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_CURRENT_SOURCE_DIR}/generate_translations_target.cmake)
generate_translations_target(translations ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/generated)
generate_translations_target(replace-translations ${CMAKE_CURRENT_SOURCE_DIR}/tests/moctranslations ${CMAKE_CURRENT_BINARY_DIR}/tests/generated)

mz_add_new_module(
    TARGET_NAME mz_translations
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/i18nstrings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/languagei18n.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/languagei18n.h
        ${CMAKE_CURRENT_SOURCE_DIR}/localizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/localizer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serveri18n.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/serveri18n.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlanguagei18n.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlanguagei18n.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlocalizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlocalizer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testserveri18n.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testserveri18n.h
    QT_DEPENDENCIES
        Core
        Qml
        Widgets
    MZ_DEPENDENCIES
        mz_telemetry
        mz_logging
        mz_context
        mz_settings
        mz_utilities
        translations
    TEST_DEPENDENCIES
        replace-translations
)
