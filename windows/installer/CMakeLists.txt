# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(WIX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/.deps/bin)
set(WIX_CANDLE_FLAGS -nologo -ext WiXUtilExtension)
set(WIX_LIGHT_FLAGS -nologo -spdb -ext WixUtilExtension)
set(WIX_PLATFORM x64)

## Download and extract Wix and friends.
file(DOWNLOAD https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311-binaries.zip
    ${CMAKE_CURRENT_BINARY_DIR}/.deps/wix-binaries.zip
    EXPECTED_HASH SHA256=2c1888d5d1dba377fc7fa14444cf556963747ff9a0a289a3599cf09da03b9e2e)

file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/.deps/wix-binaries.zip DESTINATION ${WIX_BINARY_DIR})


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/staging)
add_custom_target(msi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/staging
    COMMAND ${CMAKE_COMMAND} -E echo "Building MSI installer for $<CONFIG>"
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_CURRENT_BINARY_DIR}/staging --config $<CONFIG>
    COMMAND ${WIX_BINARY_DIR}/candle ${WIX_CANDLE_FLAGS} -dPlatform=${WIX_PLATFORM} 
                    -out ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.wixobj -arch x64 ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPN_cmake.wxs
    COMMAND ${WIX_BINARY_DIR}/light ${WIX_LIGHT_FLAGS} -out ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.msi ${CMAKE_CURRENT_BINARY_DIR}/MozillaVPN.wixobj
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/staging)
