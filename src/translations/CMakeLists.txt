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

    execute_process(
        COMMAND
            ${PYTHON_EXECUTABLE}
            ${GENERATE_XCSTRINGS_SCRIPT}
            ${CMAKE_SOURCE_DIR}/src/translations/strings.yaml
            ${CMAKE_SOURCE_DIR}/3rdparty/i18n
            ${TRANSLATIONS_OUT_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE apple_translations_result
    )
    if(NOT apple_translations_result EQUAL 0)
        message(FATAL_ERROR "xcstrings generation failed: ${apple_translations_result}")
    endif()

    set(APPLE_TRANSLATION_FILES
        ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
    )
    target_sources(mozillavpn PRIVATE
        ${APPLE_TRANSLATION_FILES}
    )
    set_property(TARGET mozillavpn APPEND PROPERTY
        RESOURCE ${APPLE_TRANSLATION_FILES}
    )

endif()
