# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/cmake/generate_translations_target.cmake)

generate_translations_target(translations
    ${CMAKE_SOURCE_DIR}/src/translations
    ${CMAKE_SOURCE_DIR}/3rdparty/i18n
)

if(IOS)
    set(GENERATE_XCSTRINGS_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/utils/generate_xcstrings.py)
    if(NOT EXISTS ${GENERATE_XCSTRINGS_SCRIPT})
        message(FATAL_ERROR "generate_xcstrings.py not found at ${GENERATE_XC_STRINGS_SCRIPT}")
    endif()

    set(TRANSLATIONS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    file(MAKE_DIRECTORY "${TRANSLATIONS_OUT_DIR}")

    file(GLOB_RECURSE XLIFF_FILE_LIST CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/3rdparty/i18n/*/mozillavpn.xliff
    )

    add_custom_command(
        OUTPUT ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
        DEPENDS ${XLIFF_FILE_LIST}
        COMMAND ${PYTHON_EXECUTABLE}
            ${GENERATE_XCSTRINGS_SCRIPT}
            ${CMAKE_SOURCE_DIR}/src/translations/strings.yaml
            ${CMAKE_SOURCE_DIR}/3rdparty/i18n
            ${TRANSLATIONS_OUT_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    set_source_files_properties(
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
        ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
        PROPERTIES GENERATED TRUE
    )

    add_custom_target(generate_xcstrings
        DEPENDS
            ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
            ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
    )
    add_dependencies(mozillavpn generate_xcstrings)

endif()
