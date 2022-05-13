# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Stage 0: Generate the Resources for the installer package.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
set(XLIFFTOOL ${CMAKE_SOURCE_DIR}/scripts/utils/xlifftool.py)
add_custom_target(pkg_resources
    COMMENT "Preparing MacOS Installer Resources"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources .
)

add_custom_target(pkg_resources_en
    COMMENT "Translating MacOS Installer Resources for en"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
    DEPENDS ${CMAKE_SOURCE_DIR}/i18n/en/mozillavpn.xliff
    COMMAND ${CMAKE_COMMAND} -E make_directory en.lproj
    COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/welcome.html.in -o en.lproj/welcome.html ${CMAKE_SOURCE_DIR}/i18n/en/mozillavpn.xliff
    COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/conclusion.html.in -o en.lproj/conclusion.html ${CMAKE_SOURCE_DIR}/i18n/en/mozillavpn.xliff
    COMMAND ${XLIFFTOOL} -g macosinstaller --strip 1 -f macos -o en.lproj/Localizable.strings ${CMAKE_SOURCE_DIR}/i18n/en/mozillavpn.xliff
)
add_dependencies(pkg_resources pkg_resources_en)

## Stage 1: Create the staging directory, and populate it with the app.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/MozillaVPN.pkg
    DEPENDS mozillavpn
    COMMAND ${CMAKE_COMMAND} -E remove_directory -f ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:mozillavpn>
       ${CMAKE_CURRENT_BINARY_DIR}/staging/Applications/$<TARGET_PROPERTY:mozillavpn,OUTPUT_NAME>.app/
    COMMAND pkgbuild --identifier "$<TARGET_PROPERTY:mozillavpn,MACOSX_BUNDLE_GUI_IDENTIFIER>" --version "2.0"
        --scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts --root ${CMAKE_CURRENT_BINARY_DIR}/staging
        ${CMAKE_CURRENT_BINARY_DIR}/staging/MozillaVPN.pkg
)

## Stage 2: Run productbuild to inject the distribution files.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN-unsigned.pkg
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/staging/MozillaVPN.pkg pkg_resources
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND productbuild --distribution ${CMAKE_CURRENT_SOURCE_DIR}/Distribution
        --resources ${CMAKE_CURRENT_BINARY_DIR}/Resources --package-path staging
        MozillaVPN-unsigned.pkg
)

## Stage 3 (optional): Sign the installer package
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
