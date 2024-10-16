# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Common files/directories
get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts ABSOLUTE)

## Lookup the path to the Qt linguist tools
## CMake support for the LinquistTools component appears to be broken,
## so instead we will workaround it by searching the path where other
## tools can be found.
get_target_property(QT_QMLLINT_EXECUTABLE Qt6::qmllint LOCATION)
get_filename_component(QT_TOOL_PATH ${QT_QMLLINT_EXECUTABLE} PATH)
find_program(QT_LCONVERT_EXECUTABLE
    NAMES lconvert-qt6 lconvert lconvert-qt5
    PATHS ${QT_TOOL_PATH}
    NO_DEFAULT_PATH)
find_program(QT_LUPDATE_EXECUTABLE
    NAMES lupdate-qt6 lupdate lupdate-qt5
    PATHS ${QT_TOOL_PATH}
    NO_DEFAULT_PATH)
find_program(QT_LRELEASE_EXECUTABLE
    NAMES lrelease-qt6 lrelease lrelease-qt5
    PATHS ${QT_TOOL_PATH}
    NO_DEFAULT_PATH)

function(generate_translations_target TARGET_NAME ASSETS_DIRECTORY TRANSLATIONS_DIRECTORY)
    ## The generated folder will contain sub-folders for each supported project
    get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)


    message("Creating library ${TARGET_NAME} from ${ASSETS_DIRECTORY}")
    add_library(${TARGET_NAME} STATIC)

    mz_target_handle_warnings(${TARGET_NAME})

    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Libs")
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core Qt6::Qml)

    file(MAKE_DIRECTORY ${GENERATED_DIR})
    target_include_directories(${TARGET_NAME} PUBLIC ${GENERATED_DIR})

    target_sources(${TARGET_NAME} PRIVATE
        ${GENERATED_DIR}/i18nlanguagenames.h
        ${GENERATED_DIR}/i18nstrings_p.cpp
        ${GENERATED_DIR}/i18nstrings.h
        ${GENERATED_DIR}/translations.qrc
        ${CMAKE_SOURCE_DIR}/translations/i18nstrings.cpp
    )

    if(NOT QT_FEATURE_zstd)
        set_property(SOURCE ${GENERATED_DIR}/translations.qrc PROPERTY AUTORCC_OPTIONS "--no-zstd")
    endif()

    ## Generate the string database (language agnostic)
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/i18nstrings_p.cpp ${GENERATED_DIR}/i18nstrings.h
        DEPENDS
            ${ASSETS_DIRECTORY}/strings.yaml
            ${ASSETS_DIRECTORY}/extras/extras.xliff
            ${MVPN_SCRIPT_DIR}/utils/generate_strings.py
        # Next line fixes VPN-6649 - Malmö is the only city with a special character. The translation build scripts should
        # handle this fine, and it built perfectly on a developer's machine. However on TaskCluster it built differently,
        # leading to this bug. Adding this `sed` command here and below fixes it.
        COMMAND sed -i -- 's/servers\.Malmö/servers\.Malm/g' ${ASSETS_DIRECTORY}/extras/extras.xliff
        COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_strings.py
            -o ${GENERATED_DIR}
            ${ASSETS_DIRECTORY}/strings.yaml
            ${ASSETS_DIRECTORY}/extras/extras.xliff
    )

    add_custom_command(
        OUTPUT ${GENERATED_DIR}/i18nlanguagenames.h
        DEPENDS
            ${ASSETS_DIRECTORY}/extras/extras.xliff
            ${MVPN_SCRIPT_DIR}/utils/generate_language_names_map.py
        COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_language_names_map.py
            ${TRANSLATIONS_DIRECTORY}
            ${GENERATED_DIR}/i18nlanguagenames.h
    )

    ## Build the list of supported locales and add rules to build them.
    file(GLOB I18N_LOCALES LIST_DIRECTORIES true RELATIVE ${TRANSLATIONS_DIRECTORY} ${TRANSLATIONS_DIRECTORY}/*)
    list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
    foreach(LOCALE ${I18N_LOCALES})
        if(NOT EXISTS ${TRANSLATIONS_DIRECTORY}/${LOCALE}/mozillavpn.xliff)
            list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
            continue()
        endif()

        if(${LOCALE} STREQUAL "en")
            set(INCLUDE_UNTRANSLATED "")
        else()
            set(INCLUDE_UNTRANSLATED "-no-untranslated")
        endif()

        add_custom_command(
            OUTPUT ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
            MAIN_DEPENDENCY ${TRANSLATIONS_DIRECTORY}/${LOCALE}/mozillavpn.xliff
            DEPENDS
                ${GENERATED_DIR}/i18nstrings_p.cpp
                ${TRANSLATIONS_DIRECTORY}/${LOCALE}/extras.xliff
            COMMAND ${QT_LUPDATE_EXECUTABLE} -target-language ${LOCALE} ${GENERATED_DIR}/i18nstrings_p.cpp -ts ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
            # Next line fixes VPN-6649 - See comment above. Adding `sed` command here and above fixes bug.
            COMMAND sed -i -- 's/servers\.Malmö/servers\.Malm/g' "${TRANSLATIONS_DIRECTORY}/${LOCALE}/extras.xliff"
            COMMAND ${QT_LCONVERT_EXECUTABLE} -verbose -o ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
                            -if ts -i ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts ${INCLUDE_UNTRANSLATED}
                            -if xlf -i ${TRANSLATIONS_DIRECTORY}/${LOCALE}/mozillavpn.xliff -i ${TRANSLATIONS_DIRECTORY}/${LOCALE}/extras.xliff
        )

        add_custom_command(
            OUTPUT ${GENERATED_DIR}/mozillavpn_${LOCALE}.qm
            MAIN_DEPENDENCY ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
            COMMAND ${QT_LRELEASE_EXECUTABLE} -verbose -idbased ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
        )
    endforeach()

    ## Generate the translation resource file.
    ## TODO: This should be a build-time command that depends on the input XLIFFs.
    file(WRITE ${GENERATED_DIR}/translations.qrc "<RCC>\n    <qresource prefix=\"/i18n\">\n")

    if (NOT EXISTS ${ASSETS_DIRECTORY}/extras/translations.completeness)
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file>translations.completeness</file>\n")
        file(REMOVE ${GENERATED_DIR}/translations.completeness)
    endif()

    foreach(LOCALE ${I18N_LOCALES})
        execute_process(
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE I18N_COMPLETENESS
            COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/xlifftool.py -C --locale=${LOCALE} ${TRANSLATIONS_DIRECTORY}/${LOCALE}/mozillavpn.xliff
        )
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file>mozillavpn_${LOCALE}.qm</file>\n")

        if (NOT EXISTS ${ASSETS_DIRECTORY}/extras/translations.completeness)
            file(APPEND ${GENERATED_DIR}/translations.completeness "${LOCALE}:${I18N_COMPLETENESS}\n")
            message("Importing translations: ${LOCALE} - completeness: ${I18N_COMPLETENESS}")
        endif()
    endforeach()

    ## Copy the extra files
    get_filename_component(EXTRAS_DIR ${ASSETS_DIRECTORY}/extras ABSOLUTE)
    file(GLOB EXTRAS_FILES LIST_DIRECTORIES true RELATIVE ${EXTRAS_DIR} ${EXTRAS_DIR}/*)
    list(FILTER EXTRAS_FILES EXCLUDE REGEX "^\\..+")
    foreach(EXTRA ${EXTRAS_FILES})
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file alias=\"${EXTRA}\">${ASSETS_DIRECTORY}/extras/${EXTRA}</file>\n")
    endforeach()

    # In case the translations.completeness is still missing (does i18n folder
    # exist?), let's create it now.
    if (NOT EXISTS ${ASSETS_DIRECTORY}/extras/translations.completeness AND
        NOT EXISTS ${GENERATED_DIR}/translations.completeness)
        message("Creating an empty tanslations.completeness file")
        file(WRITE ${GENERATED_DIR}/translations.completeness "")
    endif()

    file(APPEND ${GENERATED_DIR}/translations.qrc "    </qresource>\n</RCC>\n")
endfunction()
