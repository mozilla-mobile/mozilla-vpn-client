# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.18)

project(msi
    VERSION 1.0.0
    LANGUAGES NONE
    DESCRIPTION "Mozilla VPN installer"
)

# Determine the root of the project checkout - we need scripts and localization.
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
else()
    get_filename_component(PROJECT_ROOT_DIR ../.. ABSOLUTE)
    find_program(PYTHON_EXECUTABLE NAMES python3 python)
endif()

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

# Determine the MSI installer arch by reading the PE/COFF header
function(__read_pe_arch FILENAME OUTVAR)
    # Find the PE header offset
    file(READ ${FILENAME} LOW_BYTE OFFSET "60" LIMIT 1 HEX)
    file(READ ${FILENAME} HIGH_BYTE OFFSET "61" LIMIT 1 HEX)
    math(EXPR PE_SIG_OFFSET "0x${HIGH_BYTE}${LOW_BYTE}" OUTPUT_FORMAT DECIMAL)
    math(EXPR PE_MACH_OFFSET "${PE_SIG_OFFSET} + 4" OUTPUT_FORMAT DECIMAL)

    # Read the signature, which must contain the string "PE/0/0"
    file(READ ${FILENAME} PE_SIGNATURE OFFSET ${PE_SIG_OFFSET} LIMIT 4 HEX)
    if(NOT "${PE_SIGNATURE}" STREQUAL "50450000")
        message(WARNING "Binary file ${FILENAME} does not appear to be a PE/COFF executable")
        set(${OUTVAR} "x64" PARENT_SCOPE)
        return()
    endif()

    # Read the COFF machine type
    file(READ ${FILENAME} PE_MACH_LOW OFFSET ${PE_MACH_OFFSET} LIMIT 1 HEX)
    math(EXPR PE_MACH_OFFSET "${PE_MACH_OFFSET} + 1" OUTPUT_FORMAT DECIMAL)
    file(READ ${FILENAME} PE_MACH_HIGH OFFSET ${PE_MACH_OFFSET} LIMIT 1 HEX)
    set(PE_MACH_TYPE "${PE_MACH_HIGH}${PE_MACH_LOW}")

    # See: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#machine-types
    if(PE_MACH_TYPE STREQUAL "8664")
        set(${OUTVAR} "x64" PARENT_SCOPE)
    elseif(PE_MACH_TYPE STREQUAL "aa64")
        set(${OUTVAR} "arm64" PARENT_SCOPE)
    elseif(PE_MACH_TYPE STREQUAL "014c")
        set(${OUTVAR} "x86" PARENT_SCOPE)
    else()
        message(WARNING "Binary file ${FILENAME} has unknown machine type ${PE_MACH_TYPE}")
        set(${OUTVAR} "x64" PARENT_SCOPE)
    endif()
endfunction()

# Run CMake install to put everything together into the staging directory.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # If this is the top-level project, use CMAKE_BINARY_DIR as the staging dir.
    set(WIX_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR})
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt)

    # Select the MSI installer architecture from the executable being installed.
    __read_pe_arch("${WIX_STAGING_DIR}/Mozilla VPN.exe" WIX_PLATFORM)
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

    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" WIX_PLATFORM)
    if(WIX_PLATFORM STREQUAL "amd64")
        set(WIX_PLATFORM "x64")
    endif()
endif()

# Install the Wix util extension.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Installing WixToolset.Util.wixext extension"
    COMMAND ${WIX_BUILD_TOOL} extension add WixToolset.Util.wixext/${WIX_BUILD_VERSION}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
)

# Build the localization sources
file(GLOB I18N_LOCALES LIST_DIRECTORIES true
    RELATIVE ${PROJECT_ROOT_DIR}/3rdparty/i18n ${PROJECT_ROOT_DIR}/3rdparty/i18n/*)
list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")

set(WIX_I18N_ARGS)
set(WIX_I18N_DEPS)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/i18n)
foreach(LOCALE ${I18N_LOCALES})
    if(NOT IS_DIRECTORY ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE})
        list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
        continue()
    endif()
    if(NOT EXISTS ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
        list(REMOVE_ITEM I18N_LOCALES ${LOCALE})
        continue()
    endif()
    set(XLIFFTOOL ${PYTHON_EXECUTABLE} ${PROJECT_ROOT_DIR}/scripts/utils/xlifftool.py ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff)
    execute_process(
        RESULT_VARIABLE XLIFF_CHECK_RESULT
        COMMAND ${XLIFFTOOL} --locale=${LOCALE} --check
    )
    if(NOT XLIFF_CHECK_RESULT EQUAL 0)
        continue()
    endif()

    # Build the localized wxl file
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-${LOCALE}.wxl
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxl.in
            ${PROJECT_ROOT_DIR}/3rdparty/i18n/${LOCALE}/mozillavpn.xliff
        COMMAND ${XLIFFTOOL} -x ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxl.in -o ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-${LOCALE}.wxl
    )

    list(APPEND WIX_I18N_DEPS ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-${LOCALE}.wxl)
    list(APPEND WIX_I18N_ARGS -loc ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-${LOCALE}.wxl)
endforeach()

# Wix translations require exactly one language file to be the native locale,
# which should have the XML Culture attribute removed. Use the 'en' locale.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-en.wxl APPEND
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fixup-native-locale.py
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fixup-native-locale.py ${CMAKE_CURRENT_BINARY_DIR}/i18n/MozillaVPN-en.wxl
)

# Append the architecture to the filename so that we can distinguish the
# build artifacts when uploaded to a common directory.
if(WIX_PLATFORM STREQUAL "x64")
    set(MSI_OUTPUT_FILENAME MozillaVPN.msi)
elseif(WIX_PLATFORM STREQUAL "arm64")
    set(MSI_OUTPUT_FILENAME MozillaVPN-aarch64.msi)
else()
    set(MSI_OUTPUT_FILENAME "MozillaVPN-${WIX_PLATFORM}.msi")
endif()

# Repack the CRT merge module to workaround arch mismatches.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Microsoft_CRT_repack.msm
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Microsoft_CRT_repack.wxs
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt
    COMMAND ${CMAKE_COMMAND} -E echo "Repack CRT merge module"
    COMMAND ${WIX_BUILD_TOOL} msi decompile
                -out ${CMAKE_CURRENT_BINARY_DIR}/Microsoft_CRT_repack.wxs
                -x ${CMAKE_CURRENT_BINARY_DIR}/SourceDir
                ${WIX_STAGING_DIR}/Microsoft_CRT_x64.msm
    COMMAND ${WIX_BUILD_TOOL} build -arch ${WIX_PLATFORM} ${CMAKE_CURRENT_BINARY_DIR}/Microsoft_CRT_repack.wxs
)

# Build the MSI installer package
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MSI_OUTPUT_FILENAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxs
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/install-stamp.txt
        ${CMAKE_CURRENT_BINARY_DIR}/wixext-stamp.txt
        ${CMAKE_CURRENT_BINARY_DIR}/Microsoft_CRT_repack.msm
        ${WIX_I18N_DEPS}
    COMMAND ${CMAKE_COMMAND} -E echo "Building MSI installer"
    COMMAND ${WIX_BUILD_TOOL} build ${WIX_I18N_ARGS}
                    -d Platform=${WIX_PLATFORM} -arch ${WIX_PLATFORM}
                    -bindpath ${WIX_STAGING_DIR} -ext WixToolset.Util.wixext
                    -out ${CMAKE_CURRENT_BINARY_DIR}/${MSI_OUTPUT_FILENAME}
                    ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN.wxs
)

# Wrap it up in a custom target
add_custom_target(msi DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${MSI_OUTPUT_FILENAME})
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(msi PROPERTIES EXCLUDE_FROM_ALL FALSE)
endif()

# Make it installable as the msi component
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${MSI_OUTPUT_FILENAME}
    DESTINATION .
    COMPONENT msi
    EXCLUDE_FROM_ALL
)
