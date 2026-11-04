# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)

# Build the Wireguard Go tunnel
file(GLOB_RECURSE WIREGUARD_DEPS ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/*.go)
set(WIREGUARD_BUILD_ENV
    GOCACHE=${CMAKE_BINARY_DIR}/go-cache
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
    GOROOT=${GOLANG_GOROOT}
    MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    GOOS=darwin
    CGO_ENABLED=1
    GO111MODULE=on
)
if (CMAKE_CROSSCOMPILING)
    list(APPEND WIREGUARD_BUILD_ENV
        CGO_CFLAGS='-target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
        CGO_LDFLAGS='-target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
    )
endif()

# Find the SDK root
if(IS_DIRECTORY ${CMAKE_OSX_SYSROOT})
    list(APPEND WIREGUARD_BUILD_ENV SDKROOT=${CMAKE_OSX_SYSROOT})
elseif(CMAKE_OSX_SYSROOT)
    execute_process(OUTPUT_VARIABLE OSX_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path)
    list(APPEND WIREGUARD_BUILD_ENV SDKROOT=${OSX_SDK_PATH})
else()
    execute_process(OUTPUT_VARIABLE OSX_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND xcrun --sdk macosx --show-sdk-path)
    list(APPEND WIREGUARD_BUILD_ENV SDKROOT=${OSX_SDK_PATH})
endif()

# Prepare an overlay to patch setMTU for darwin.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tun_darwin_patch.go
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/0001-darwin-setmtu-noop.patch
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
    COMMAND patch -f -p1
        -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-darwin-setmtu-noop.patch
        -o ${CMAKE_CURRENT_BINARY_DIR}/tun_darwin_patch.go
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/overlay.json.in ${CMAKE_CURRENT_BINARY_DIR}/overlay.json)
list(APPEND WIREGUARD_DEPS ${CMAKE_CURRENT_BINARY_DIR}/tun_darwin_patch.go)

# Build wireguard-go for each target architecture
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(WIREGUARD_TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
else()
    set(WIREGUARD_TARGET_ARCH ${CMAKE_OSX_ARCHITECTURES})
endif()
foreach(OSXARCH ${WIREGUARD_TARGET_ARCH})
    string(REPLACE "x86_64" "amd64" GOARCH ${OSXARCH})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
        COMMENT "Building wireguard-go for ${OSXARCH}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
        DEPENDS
            ${WIREGUARD_DEPS}
            ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.mod
            ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.sum
            ${CMAKE_CURRENT_BINARY_DIR}/overlay.json
        COMMAND ${CMAKE_COMMAND} -E env ${WIREGUARD_BUILD_ENV} GOARCH=${GOARCH}
                ${GOLANG_BUILD_TOOL} build -buildmode exe -buildvcs=false -trimpath -v -ldflags='-s -w'
                    -overlay ${CMAKE_CURRENT_BINARY_DIR}/overlay.json
                    -o ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
    )
    list(APPEND WIREGUARD_TARGET_ARTIFACTS ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH})
endforeach()

# Create a dummy executable to drive the build and make ourselves look like an executable target.
add_executable(wireguard-go dummy.c ${WIREGUARD_TARGET_ARTIFACTS})
add_custom_command(TARGET wireguard-go POST_BUILD
    COMMENT "Merging wireguard-go"
    COMMAND ${LIPO_BUILD_TOOL} -create -output $<TARGET_FILE:wireguard-go> ${WIREGUARD_TARGET_ARTIFACTS}
)
set_source_files_properties(${WIREGUARD_TARGET_ARTIFACTS} PROPERTIES
    HEADER_FILE_ONLY TRUE
    GENERATED TRUE
)
set_target_properties(wireguard-go PROPERTIES
    OUTPUT_NAME ${BUILD_OSX_APP_IDENTIFIER}.wireguard
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.wireguard"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${CMAKE_PROJECT_VERSION}"
    LINK_DEPENDS ${WIREGUARD_TARGET_ARTIFACTS}
    FOLDER "Tools"
)

# Sign the wireguard-go binary.
if(XCODE)
    # We need to manually sign, because our executable overwrite hack will blow away the Xcode signature.
    add_custom_command(TARGET wireguard-go POST_BUILD
        COMMENT "Signing wireguard-go"
        COMMAND codesign --timestamp -f 
            -s $<QUOTE>$EXPANDED_CODE_SIGN_IDENTITY$<QUOTE>
            -i ${BUILD_OSX_APP_IDENTIFIER}.wireguard
            $<TARGET_FILE:wireguard-go>
    )
else()
    osx_codesign_target(wireguard-go)
endif()
