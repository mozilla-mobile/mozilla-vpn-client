# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Stage 0: Generate the Resources for the installer package.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
add_custom_target(pkg_resources
    COMMENT "Preparing MacOS Installer Resources"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources .
)
set_target_properties(pkg_resources PROPERTIES FOLDER "Installer")

file(GLOB I18N_LOCALES LIST_DIRECTORIES true
    RELATIVE ${CMAKE_SOURCE_DIR}/3rdparty/i18n ${CMAKE_SOURCE_DIR}/3rdparty/i18n/*)
list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
foreach(LOCALE ${I18N_LOCALES})
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
        list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
        continue()
    endif()
    set(XLIFFTOOL ${CMAKE_SOURCE_DIR}/scripts/utils/xlifftool.py ${CMAKE_SOURCE_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
    execute_process(
        RESULT_VARIABLE XLIFF_CHECK_RESULT
        COMMAND ${XLIFFTOOL} --locale=${LOCALE} --check
    )
    if(NOT XLIFF_CHECK_RESULT EQUAL 0)
        continue()
    endif()

    add_custom_target(pkg_resources_${LOCALE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
        SOURCES
            ${CMAKE_SOURCE_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff
            ${CMAKE_CURRENT_SOURCE_DIR}/welcome.html.in
            ${CMAKE_CURRENT_SOURCE_DIR}/conclusion.html.in
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCALE}.lproj
        COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/welcome.html.in -o ${LOCALE}.lproj/welcome.html
        COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/conclusion.html.in -o ${LOCALE}.lproj/conclusion.html
        COMMAND ${XLIFFTOOL} -g macosinstaller --strip 1 -f macos -o ${LOCALE}.lproj/Localizable.strings
    )
    add_dependencies(pkg_resources pkg_resources_${LOCALE})
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/Resources/${LOCALE}.lproj)
    set_target_properties(pkg_resources_${LOCALE} PROPERTIES FOLDER "Installer")
endforeach()

## Stage 1: Preprocess the installation scripts.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/postinstall.in
    ${CMAKE_CURRENT_BINARY_DIR}/scripts/postinstall
    @ONLY
)

## Stage 2: Create the staging directory and build the component package.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    DEPENDS mozillavpn
    COMMAND ${CMAKE_COMMAND} -E remove_directory -f ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/components
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:mozillavpn>
       ${CMAKE_CURRENT_BINARY_DIR}/staging/$<TARGET_PROPERTY:mozillavpn,OUTPUT_NAME>.app/
    COMMAND pkgbuild --analyze --root ${CMAKE_CURRENT_BINARY_DIR}/staging ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    COMMAND plutil -replace 0.BundleIsRelocatable -bool NO ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
    COMMAND pkgbuild --identifier "$<TARGET_PROPERTY:mozillavpn,XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER>" 
        --version "2.0" --install-location "/Applications"
        --scripts ${CMAKE_CURRENT_BINARY_DIR}/scripts --root ${CMAKE_CURRENT_BINARY_DIR}/staging
        --component-plist ${CMAKE_CURRENT_BINARY_DIR}/installer.plist
        ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg
)

## Stage 3: Run productbuild to inject the distribution files and build the installer package.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-unsigned.pkg
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/components/MozillaVPN.pkg pkg_resources
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND productbuild --distribution ${CMAKE_CURRENT_SOURCE_DIR}/Distribution
        --resources ${CMAKE_CURRENT_BINARY_DIR}/Resources --package-path ${CMAKE_CURRENT_BINARY_DIR}/components
        MozillaVPN-unsigned.pkg
)

## Stage 4 (optional): Sign the installer package
if(INSTALLER_SIGN_IDENTITY)
    add_custom_command(
        COMMENT "Building Signed MacOS Installer Package"
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-signed.pkg
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-unsigned.pkg
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
        COMMAND productsign --sign "${INSTALLER_SIGN_IDENTITY}"
            MozillaVPN-unsigned.pkg MozillaVPN-signed.pkg
    )
    add_custom_target(pkg DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-signed.pkg)
else()
    add_custom_target(pkg DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-unsigned.pkg)
endif()
set_target_properties(pkg PROPERTIES
    FOLDER "Installer"
    XCODE_GENERATE_SCHEME TRUE
)
