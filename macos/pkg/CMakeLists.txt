# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.18)

project(mozillavpn-macpkg VERSION 1.0.0
        DESCRIPTION "MacOS Package for the Mozilla VPN"
)

# Determine the root of the project checkout - we need scripts and localization.
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
else()
    get_filename_component(PROJECT_ROOT_DIR ../.. ABSOLUTE)
endif()

# Set the default MACPKG_SOURCE_BUNDLE to the mozillavpn target
if(NOT DEFINED MACPKG_SOURCE_BUNDLE)
    if(TARGET mozillavpn)
        set(MACPKG_SOURCE_BUNDLE mozillavpn)
    else()
        message(FATAL_ERROR "Standalone macOS package build requires MACPKG_SOURCE_BUNDLE to be set")
    endif()
endif()

if(NOT BUILD_OSX_APP_IDENTIFIER)
    if(IS_DIRECTORY ${MACPKG_SOURCE_BUNDLE})
        execute_process(
            OUTPUT_VARIABLE BUILD_OSX_APP_IDENTIFIER
            OUTPUT_STRIP_TRAILING_WHITESPACE
            COMMAND plutil -extract CFBundleIdentifier raw ${MACPKG_SOURCE_BUNDLE}/Contents/Info.plist
        )
    else()
        message(FATAL_ERROR "Unable to determine macOS app bundle identifier")
    endif()
endif()
if(NOT BUILD_VPN_DEVELOPMENT_TEAM)
    if(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM)
        set(BUILD_VPN_DEVELOPMENT_TEAM ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
    else()
        set(BUILD_VPN_DEVELOPMENT_TEAM 43AQ936H96 CACHE STRING "Mozilla VPN Development Team")
    endif()
endif()

## Stage 1: Preprocess the installation scripts.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/postinstall.in
    ${CMAKE_CURRENT_BINARY_DIR}/scripts/postinstall
    @ONLY
)

## Stage 2: Create the staging directory with the app bundles.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.staging-stamp
    COMMAND ${CMAKE_COMMAND} -E remove_directory -f ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/staging
)
# How we copy the bundle depends on what the bundle is
if (TARGET ${MACPKG_SOURCE_BUNDLE})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.staging-stamp APPEND
        DEPENDS ${MACPKG_SOURCE_BUNDLE}
        COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:${MACPKG_SOURCE_BUNDLE}>
            ${CMAKE_CURRENT_BINARY_DIR}/staging/$<TARGET_PROPERTY:${MACPKG_SOURCE_BUNDLE},OUTPUT_NAME>.app
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/.staging-stamp
    )
elseif(IS_DIRECTORY ${MACPKG_SOURCE_BUNDLE})
    file(TO_CMAKE_PATH ${MACPKG_SOURCE_BUNDLE} MACPKG_SOURCE_PATH)
    get_filename_component(MACPKG_BUNDLE_NAME ${MACPKG_SOURCE_PATH} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.staging-stamp APPEND
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MACPKG_SOURCE_PATH}
            "${CMAKE_CURRENT_BINARY_DIR}/staging/${MACPKG_BUNDLE_NAME}"
    )
    # Note: Don't create the stamp, this forces a rebuild each time we run it.
else()
    message(AUTHOR_WARNING "MACPKG_SOURCE_BUNDLE is neither a target nor directory")
endif()

## Stage 2: Create the staging directory and build the component package.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/.staging-stamp
        ${CMAKE_CURRENT_BINARY_DIR}/scripts/postinstall
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/components
    COMMAND pkgbuild --analyze --root ${CMAKE_CURRENT_BINARY_DIR}/staging ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    COMMAND plutil -replace 0.BundleIsRelocatable -bool NO ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    COMMAND pkgbuild --version "2.0" --install-location "/Applications"
        --scripts ${CMAKE_CURRENT_BINARY_DIR}/scripts --root ${CMAKE_CURRENT_BINARY_DIR}/staging
        --component-plist ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
        ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg
)

## Stage 3: Run productbuild to inject the distribution files and build the installer package.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.pkg
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND productbuild --distribution ${CMAKE_CURRENT_SOURCE_DIR}/Distribution
        --resources ${CMAKE_CURRENT_BINARY_DIR}/Resources --package-path ${CMAKE_CURRENT_BINARY_DIR}/components
        MozillaVPN.pkg
)

## Stage 4: Add resources to the product build.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(GLOB RESOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/Resources
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/*
)
foreach(FILENAME ${RESOURCE_FILES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Resources/${FILENAME}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${FILENAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/Resources/
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.pkg APPEND
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Resources/${FILENAME}
    )
endforeach()

# And the localized resources too.
file(GLOB I18N_LOCALES LIST_DIRECTORIES true
    RELATIVE ${PROJECT_ROOT_DIR}/3rdparty/i18n ${PROJECT_ROOT_DIR}/3rdparty/i18n/*)
list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
foreach(LOCALE ${I18N_LOCALES})
    if(NOT EXISTS ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
        list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
        continue()
    endif()
    set(XLIFFTOOL ${PROJECT_ROOT_DIR}/scripts/utils/xlifftool.py ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
    execute_process(
        RESULT_VARIABLE XLIFF_CHECK_RESULT
        COMMAND ${XLIFFTOOL} --locale=${LOCALE} --check
    )
    if(NOT XLIFF_CHECK_RESULT EQUAL 0)
        continue()
    endif()

    # Build the localized file
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/Localizable.strings
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/conclusion.html
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/welcome.html
        DEPENDS
            ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff
            ${CMAKE_CURRENT_SOURCE_DIR}/conclusion.html.in
            ${CMAKE_CURRENT_SOURCE_DIR}/welcome.html.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCALE}.lproj
        COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/welcome.html.in -o ${LOCALE}.lproj/welcome.html
        COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/conclusion.html.in -o ${LOCALE}.lproj/conclusion.html
        COMMAND ${XLIFFTOOL} -g macosinstaller --strip 1 -f macos -o ${LOCALE}.lproj/Localizable.strings
    )
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj)

    # Add as dependencies on the installer package.
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.pkg APPEND
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/Localizable.strings
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/conclusion.html
            ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj/welcome.html
    )
endforeach()

# Add a custom target to drive package creation
add_custom_target(macpkg DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.pkg)
set_target_properties(macpkg PROPERTIES
    FOLDER "Installer"
    XCODE_GENERATE_SCHEME TRUE
)
# If this is the top level project, build the macpkg by default
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(macpkg PROPERTIES EXCLUDE_FROM_ALL FALSE)
endif()
