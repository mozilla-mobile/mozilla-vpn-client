# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(translations STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(translations PRIVATE Qt6::Core Qt6::Qml)

get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../scripts ABSOLUTE)
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated ABSOLUTE)
file(MAKE_DIRECTORY ${GENERATED_DIR})
target_include_directories(translations PUBLIC ${GENERATED_DIR})

target_sources(translations PRIVATE
    ${GENERATED_DIR}/l18nstrings_p.cpp
    ${GENERATED_DIR}/l18nstrings.h
    ${GENERATED_DIR}/translations.qrc
    l18nstrings.cpp
    servers.qrc
)

## Generate the string database (language agnostic)
add_custom_command(
    OUTPUT ${GENERATED_DIR}/l18nstrings_p.cpp ${GENERATED_DIR}/l18nstrings.h
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/strings.yaml
    COMMAND python3 ${MVPN_SCRIPT_DIR}/utils/generate_strings.py -o ${GENERATED_DIR}
                -g ${CMAKE_SOURCE_DIR}/src/ui/guides
                -t ${CMAKE_SOURCE_DIR}/src/ui/tutorials
                ${CMAKE_CURRENT_SOURCE_DIR}/strings.yaml
)

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

## Build the list of supported locales
set(I18N_LOCALES)
get_filename_component(I18N_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../i18n ABSOLUTE)
file(GLOB I18N_LISTING LIST_DIRECTORIES true RELATIVE ${I18N_DIR} ${I18N_DIR}/*)
list(FILTER I18N_LISTING EXCLUDE REGEX "^\\..+")
foreach(entry ${I18N_LISTING})
    if(IS_DIRECTORY ${I18N_DIR}/${entry})
        list(APPEND I18N_LOCALES ${entry})
    endif()
endforeach()

## Generate translation files for each locale present
file(WRITE ${GENERATED_DIR}/translations.qrc "<RCC>\n    <qresource prefix=\"/i18n\">\n")
foreach(LOCALE ${I18N_LOCALES})
    file(MAKE_DIRECTORY ${LOCALE})
    file(APPEND ${GENERATED_DIR}/translations.qrc "        <file>mozillavpn_${LOCALE}.qm</file>\n")

    add_custom_command(
        OUTPUT ${LOCALE}/locversion.plist
        MAIN_DEPENDENCY locversion.plist.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${MVPN_SCRIPT_DIR}/utils/make_template.py -k __LOCALE__=${LOCALE} -o ${LOCALE}/locversion.plist locversion.plist.in
    )

    add_custom_command(
        OUTPUT ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
        MAIN_DEPENDENCY ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
        DEPENDS ${GENERATED_DIR}/l18nstrings_p.cpp
        COMMAND ${QT_LUPDATE_EXECUTABLE} -target-language ${LOCALE} ${GENERATED_DIR}/l18nstrings_p.cpp -ts ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
        COMMAND ${QT_LCONVERT_EXECUTABLE} -verbose -o ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
                      -if xlf -i ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
                      -if ts -i ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
    )

    add_custom_command(
        OUTPUT ${GENERATED_DIR}/mozillavpn_${LOCALE}.qm
        MAIN_DEPENDENCY ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
        COMMAND ${QT_LRELEASE_EXECUTABLE} -verbose -idbased ${GENERATED_DIR}/mozillavpn_${LOCALE}.ts
    )
endforeach()
file(APPEND ${GENERATED_DIR}/translations.qrc "    </qresource>\n</RCC>\n")
