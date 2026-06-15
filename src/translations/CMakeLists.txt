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

    # `execute_process` runs when setting up the Xcode Project. This has the downside
    # of not re-running unless the iOS build folder is deleted - so new translations
    # are not added until the folder is removed.
    # Running the Python script was previously done at build time, with `execute_process` being
    # replaced by `add_custom_command`. However, that led to a bug where keys were shown for intent
    # suggestions (https://mozilla-hub.atlassian.net/browse/VPN-7603?focusedCommentId=1436443).
    # Xcode was pulling the translation files for system extensions before the command was actually
    # being run, even though the command and its output was set to be target that was a dependency
    # of the mozillavpn target. Thus, we were always building with the xcstrings files that were from
    # the prior build. Or in the case of the first build (like in CI), we weren't including them at all. 
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
    message("Output xcstrings file in ${TRANSLATIONS_OUT_DIR}")

    target_sources(mozillavpn PRIVATE
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
        ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
    )

    set_property(TARGET mozillavpn APPEND PROPERTY RESOURCE
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
        ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
    )

    # Localizable.xcstrings contains strings both for app intents (needed by mozillavpn target)
    # and widget descriptions (needed by widgetextension target).
    # Thus, it gets added to both targets.
    # Despite the Xcode project not showing it as part of both targets, it gets compiled successfully
    # into both of them.
    target_sources(widgetextension PRIVATE
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
    )

    set_property(TARGET widgetextension APPEND PROPERTY RESOURCE
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
    )

    set_source_files_properties(
        ${TRANSLATIONS_OUT_DIR}/Localizable.xcstrings
        ${TRANSLATIONS_OUT_DIR}/AppShortcuts.xcstrings
        PROPERTIES GENERATED TRUE
    )

endif()
