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

function(generate_translations_target TARGET_NAME TRANSLATIONS_DIRECTORY)
    ## The generated folder will contain sub-folders for each supported project
    get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)


    message("Creating library ${TARGET_NAME} from ${TRANSLATIONS_DIRECTORY}")
    add_library(${TARGET_NAME} STATIC)

    if(NOT MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror -Wno-conversion)
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Libs")
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core Qt6::Qml)

    file(MAKE_DIRECTORY ${GENERATED_DIR})
    target_include_directories(${TARGET_NAME} PUBLIC ${GENERATED_DIR})

    target_sources(${TARGET_NAME} PRIVATE
        ${GENERATED_DIR}/i18nstrings_p.cpp
        ${GENERATED_DIR}/i18nstrings.h
        ${GENERATED_DIR}/translations.qrc
        ${CMAKE_SOURCE_DIR}/translations/i18nstrings.cpp
    )

    ## Generate the string database (language agnostic)
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/i18nstrings_p.cpp ${GENERATED_DIR}/i18nstrings.h
        DEPENDS 
            ${TRANSLATIONS_DIRECTORY}/strings.yaml 
            ${CMAKE_SOURCE_DIR}/src/shared/translations/strings.yaml
            ${MVPN_SCRIPT_DIR}/utils/generate_strings.py
        COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_strings.py 
            -o ${GENERATED_DIR} 
            ${TRANSLATIONS_DIRECTORY}/strings.yaml 
            ${CMAKE_SOURCE_DIR}/src/shared/translations/strings.yaml
    )

    ## Build the list of supported locales and add rules to build them.
    get_filename_component(I18N_DIR ${TRANSLATIONS_DIRECTORY}/i18n ABSOLUTE)
    file(GLOB I18N_LOCALES LIST_DIRECTORIES true RELATIVE ${I18N_DIR} ${I18N_DIR}/*)
    list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
    foreach(LOCALE ${I18N_LOCALES})
        if(NOT EXISTS ${I18N_DIR}/${LOCALE}/mozillavpn.xliff)
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
            MAIN_DEPENDENCY ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
            DEPENDS ${GENERATED_DIR}/i18nstrings_p.cpp
            COMMAND ${QT_LUPDATE_EXECUTABLE} -target-language ${LOCALE} ${GENERATED_DIR}/i18nstrings_p.cpp -ts ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
            COMMAND ${QT_LCONVERT_EXECUTABLE} -verbose -o ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
                            -if ts -i ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts ${INCLUDE_UNTRANSLATED}
                            -if xlf -i ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
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

    if (NOT EXISTS ${TRANSLATIONS_DIRECTORY}/extras/translations.completeness)
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file>translations.completeness</file>\n")
        file(REMOVE ${GENERATED_DIR}/translations.completeness)
    endif()

    foreach(LOCALE ${I18N_LOCALES})
        execute_process(
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE I18N_COMPLETENESS
            COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/xlifftool.py -C --locale=${LOCALE} ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
        )
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file>mozillavpn_${LOCALE}.qm</file>\n")

        if (NOT EXISTS ${TRANSLATIONS_DIRECTORY}/extras/translations.completeness)
            file(APPEND ${GENERATED_DIR}/translations.completeness "${LOCALE}:${I18N_COMPLETENESS}\n")
            message("Importing translations: ${LOCALE} - completeness: ${I18N_COMPLETENESS}")
        endif()
    endforeach()

    ## Copy the extra files
    get_filename_component(EXTRAS_DIR ${TRANSLATIONS_DIRECTORY}/extras ABSOLUTE)
    file(GLOB EXTRAS_FILES LIST_DIRECTORIES true RELATIVE ${EXTRAS_DIR} ${EXTRAS_DIR}/*)
    list(FILTER EXTRAS_FILES EXCLUDE REGEX "^\\..+")
    foreach(EXTRA ${EXTRAS_FILES})
        file(APPEND ${GENERATED_DIR}/translations.qrc "        <file alias=\"${EXTRA}\">${TRANSLATIONS_DIRECTORY}/extras/${EXTRA}</file>\n")
    endforeach()

    ## Copy the shared extra files
    get_filename_component(SHARED_EXTRASS_DIR ${CMAKE_SOURCE_DIR}/src/shared/translations/extras ABSOLUTE)
    file(GLOB SHARED_EXTRASS_FILES LIST_DIRECTORIES true RELATIVE ${SHARED_EXTRASS_DIR} ${SHARED_EXTRASS_DIR}/*)
    list(FILTER SHARED_EXTRASS_FILES EXCLUDE REGEX "^\\..+")
    foreach(SHARED_EXTRAS ${SHARED_EXTRASS_FILES})
        if (NOT ${SHARED_EXTRAS} IN_LIST EXTRAS_FILES)
            file(APPEND ${GENERATED_DIR}/translations.qrc "        <file alias=\"${SHARED_EXTRAS}\">${CMAKE_SOURCE_DIR}/src/shared/translations/extras/${SHARED_EXTRAS}</file>\n")
        endif()
    endforeach()

    # In case the translations.completeness is still missing (does i18n folder
    # exist?), let's create it now.
    if (NOT EXISTS ${TRANSLATIONS_DIRECTORY}/extras/translations.completeness AND
        NOT EXISTS ${GENERATED_DIR}/translations.completeness)
        message("Creating an empty tanslations.completeness file")
        file(WRITE ${GENERATED_DIR}/translations.completeness "")
    endif()

    file(APPEND ${GENERATED_DIR}/translations.qrc "    </qresource>\n</RCC>\n")
endfunction()
