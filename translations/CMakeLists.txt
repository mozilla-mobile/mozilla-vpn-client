# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Common files/directories
get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts ABSOLUTE)

## The generated folder will contain sub-folders for each supported project
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)
file(MAKE_DIRECTORY ${GENERATED_DIR})

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

## Let's retrieve the list of projects. Foreach one, let's create a library.
get_filename_component(PROJECTS_DIR ${CMAKE_SOURCE_DIR}/src/apps ABSOLUTE)
file(GLOB PROJECTS_NAMES LIST_DIRECTORIES true RELATIVE ${PROJECTS_DIR} ${PROJECTS_DIR}/*)
foreach(PROJECT ${PROJECTS_NAMES})
    message("Creating library translations_${PROJECT}")
    add_library(translations_${PROJECT} STATIC)

    if(NOT MSVC)
        target_compile_options(translations_${PROJECT} PRIVATE -Wall -Werror -Wno-conversion)
    endif()

    set_target_properties(translations_${PROJECT} PROPERTIES FOLDER "Libs")
    target_link_libraries(translations_${PROJECT} PRIVATE Qt6::Core Qt6::Qml)

    file(MAKE_DIRECTORY ${GENERATED_DIR}/${PROJECT})
    target_include_directories(translations_${PROJECT} PUBLIC ${GENERATED_DIR}/${PROJECT})

    target_sources(translations_${PROJECT} PRIVATE
        ${GENERATED_DIR}/${PROJECT}/i18nstrings_p.cpp
        ${GENERATED_DIR}/${PROJECT}/i18nstrings.h
        ${GENERATED_DIR}/${PROJECT}/translations.qrc
        i18nstrings.cpp
    )

    set(TEMPLATE ${PROJECT})
    if(EXISTS ${PROJECTS_DIR}/${PROJECT}/translations/template)
        file(STRINGS ${PROJECTS_DIR}/${PROJECT}/translations/template TEMPLATE_CONTENT)
        string(REPLACE "," "<->" TEMPLATE ${TEMPLATE_CONTENT})
    endif()

    ## Generate the string database (language agnostic) for the current ${PROJECT}
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/${PROJECT}/i18nstrings_p.cpp ${GENERATED_DIR}/${PROJECT}/i18nstrings.h
        MAIN_DEPENDENCY ${PROJECTS_DIR}/${PROJECT}/translations/strings.yaml
        COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_strings.py -o ${GENERATED_DIR}/${PROJECT} ${PROJECTS_DIR}/${PROJECT}/translations/strings.yaml
    )

    ## Build the list of supported locales and add rules to build them.
    get_filename_component(I18N_DIR ${PROJECTS_DIR}/${PROJECT}/translations/i18n ABSOLUTE)
    file(GLOB I18N_LOCALES LIST_DIRECTORIES true RELATIVE ${I18N_DIR} ${I18N_DIR}/*)
    list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
    foreach(LOCALE ${I18N_LOCALES})
        if(NOT EXISTS ${I18N_DIR}/${LOCALE}/${TEMPLATE}.xliff)
            list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
            continue()
        endif()

        if(${LOCALE} STREQUAL "en")
           set(INCLUDE_UNTRANSLATED "")
        else()
           set(INCLUDE_UNTRANSLATED "-no-untranslated")
        endif()

        add_custom_command(
            OUTPUT ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts
            MAIN_DEPENDENCY ${I18N_DIR}/${LOCALE}/${TEMPLATE}.xliff
            DEPENDS ${GENERATED_DIR}/${PROJECT}/i18nstrings_p.cpp
            COMMAND ${QT_LUPDATE_EXECUTABLE} -target-language ${LOCALE} ${GENERATED_DIR}/${PROJECT}/i18nstrings_p.cpp -ts ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts
            COMMAND ${QT_LCONVERT_EXECUTABLE} -verbose -o ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts
                          -if ts -i ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts ${INCLUDE_UNTRANSLATED}
                          -if xlf -i ${I18N_DIR}/${LOCALE}/${TEMPLATE}.xliff
        )

        add_custom_command(
            OUTPUT ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.qm
            MAIN_DEPENDENCY ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts
            COMMAND ${QT_LRELEASE_EXECUTABLE} -verbose -idbased ${GENERATED_DIR}/${PROJECT}/${TEMPLATE}_${LOCALE}.ts
        )
    endforeach()

    ## Generate the translation resource file.
    ## TODO: This should be a build-time command that depends on the input XLIFFs.
    file(WRITE ${GENERATED_DIR}/${PROJECT}/translations.qrc "<RCC>\n    <qresource prefix=\"/i18n\">\n")

    if (NOT EXISTS ${PROJECTS_DIR}/${PROJECT}/translations/extras/translations.completeness)
        file(APPEND ${GENERATED_DIR}/${PROJECT}/translations.qrc "        <file>translations.completeness</file>\n")
        file(REMOVE ${GENERATED_DIR}/${PROJECT}/translations.completeness)
    endif()

    foreach(LOCALE ${I18N_LOCALES})
        execute_process(
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE I18N_COMPLETENESS
            COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/xlifftool.py -C --locale=${LOCALE} ${I18N_DIR}/${LOCALE}/${TEMPLATE}.xliff
        )
        file(APPEND ${GENERATED_DIR}/${PROJECT}/translations.qrc "        <file>${TEMPLATE}_${LOCALE}.qm</file>\n")

        if (NOT EXISTS ${PROJECTS_DIR}/${PROJECT}/translations/extras/translations.completeness)
            file(APPEND ${GENERATED_DIR}/${PROJECT}/translations.completeness "${LOCALE}:${I18N_COMPLETENESS}\n")
            message("Project ${PROJECT} - Importing translations: ${LOCALE} - completeness: ${I18N_COMPLETENESS}")
        endif()
    endforeach()

    ## Copy the extra files
    get_filename_component(EXTRAS_DIR ${PROJECTS_DIR}/${PROJECT}/translations/extras ABSOLUTE)
    file(GLOB EXTRAS_FILES LIST_DIRECTORIES true RELATIVE ${EXTRAS_DIR} ${EXTRAS_DIR}/*)
    list(FILTER EXTRAS_FILES EXCLUDE REGEX "^\\..+")
    foreach(EXTRA ${EXTRAS_FILES})
        file(APPEND ${GENERATED_DIR}/${PROJECT}/translations.qrc "        <file alias=\"${EXTRA}\">${PROJECTS_DIR}/${PROJECT}/translations/extras/${EXTRA}</file>\n")
    endforeach()

    # In case the translations.completeness is still missing (does i18n folder
    # exist?), let's create it now.
    if (NOT EXISTS ${PROJECTS_DIR}/${PROJECT}/translations/extras/translations.completeness AND
        NOT EXISTS ${GENERATED_DIR}/${PROJECT}/translations.completeness)
      message("Creating an empty tanslations.completeness file")
      file(WRITE ${GENERATED_DIR}/${PROJECT}/translations.completeness "")
    endif()

    file(APPEND ${GENERATED_DIR}/${PROJECT}/translations.qrc "    </qresource>\n</RCC>\n")

endforeach()
