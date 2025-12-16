# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.18)

project(msi
    VERSION 1.0.0
    DESCRIPTION "Mozilla VPN installer"
)

# Find the Wix installer tool, and get its version.
find_program(WIX_BUILD_TOOL wix)
if(WIX_BUILD_TOOL)
    execute_process(
        COMMAND ${WIX_BUILD_TOOL} --version
        OUTPUT_VARIABLE WIX_BUILD_VERSION_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    string(REGEX MATCH "^[0-9.]+" WIX_BUILD_VERSION "${WIX_BUILD_VERSION_RAW}")
    message("Found Wix toolset version ${WIX_BUILD_VERSION}")
    if(WIX_BUILD_VERSION VERSION_LESS "4.0")
        message(FATAL_ERROR "Minumum Wix toolset version 4.0 not met")
    endif()
else()
    # If Wix was not found, we cannot build the MSI installer package.
    message(STATUS "Wix toolset not found")
    return()
endif()

# Run CMake install to put everything together into the staging directory.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # If this is the top-level project, use CMAKE_BINARY_DIR as the staging dir.
    set(WIX_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR})
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt)
else()
    # If this is embedded with another project, install into the staging dir.
    set(WIX_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/staging)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${WIX_STAGING_DIR})

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt
        DEPENDS mozillavpn
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${WIX_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WIX_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${WIX_STAGING_DIR} --config $<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt
    )
endif()

# Install the Wix util extension.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Installing WixToolset.Util.wixext extension"
    COMMAND ${WIX_BUILD_TOOL} extension add WixToolset.Util.wixext/${WIX_BUILD_VERSION}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
)

# Use Wix to build the MSI installer.
set(WIX_PLATFORM x64)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.msi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxs
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt
        ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
    COMMAND ${CMAKE_COMMAND} -E echo "Building MSI installer"
    COMMAND ${WIX_BUILD_TOOL} build -d Platform=${WIX_PLATFORM} -arch ${WIX_PLATFORM}
                    -bindpath ${WIX_STAGING_DIR} -ext WixToolset.Util.wixext
                    -out ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.msi
                    ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxs
)
add_custom_target(msi DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.msi)
