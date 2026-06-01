# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set_property(TARGET mozillavpn APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../android/
)

if(QT_KNOWN_POLICY_QTP0002)
    qt_policy(SET QTP0002 OLD)
endif()

## Build the obfuscators shared library.
## This is used by Android via JNA.
include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(OBFUSCATORS_RUST_ARCH "aarch64-linux-android")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
    set(OBFUSCATORS_RUST_ARCH "armv7-linux-androideabi")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set(OBFUSCATORS_RUST_ARCH "i686-linux-android")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(OBFUSCATORS_RUST_ARCH "x86_64-linux-android")
else()
    message(FATAL_ERROR "Unsupported Android architecture for obfuscators: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(OBFUSCATORS_CARGO_ENV
    CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/obfuscators
    "RUSTFLAGS=-Ctarget-feature=-crt-static -C link-arg=-Wl,-z,max-page-size=16384 -C link-arg=-Wl,-z,common-page-size=16384"
)

add_rust_library(obfuscators
    ARCH ${OBFUSCATORS_RUST_ARCH}
    PACKAGE_DIR ${CMAKE_SOURCE_DIR}/obfuscators
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/obfuscators
    CRATE_NAME obfuscators
    CARGO_ENV ${OBFUSCATORS_CARGO_ENV}
    SHARED 1
)
target_include_directories(obfuscators INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/obfuscators
)

target_link_libraries(mozillavpn PRIVATE
    obfuscators
    Qt6::Test
    Qt6::Xml)

target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidauthenticationlistener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidauthenticationlistener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidiaphandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidnetworkwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidnotificationhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidvpnactivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidappimageprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidapplistprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidiaphandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidnetworkwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidnotificationhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidvpnactivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidappimageprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidapplistprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/linuxpingsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/linuxpingsender.h
)

get_property(OPENSSL_LIBS_DIR GLOBAL PROPERTY OPENSSL_LIBS)
get_property(OBFUSCATORS_LIB_LOCATION TARGET obfuscators PROPERTY LOCATION_${CMAKE_BUILD_TYPE})

# This property flags the build system to copy these
# shared libraries into the expected Android shared library folder.
#
# Qt requires this to be set on the "app" target.
add_dependencies(mozillavpn ndk_openssl_merged)
set_property(TARGET mozillavpn PROPERTY QT_ANDROID_EXTRA_LIBS
    ## --- PILE OF SHAME --- ##
    # android-deploy-qt is bad and randomly decides to not deploy
    # some lib's we are linking to.
    # So as long as qt is behaving bad, let's force the deployment.
    # Feel free to add libs that are breaking on your build locally.
    ${Qt6_DIR}/../../libQt6Test_${ANDROID_ABI}.so
    ${Qt6_DIR}/../../libQt6WebSockets_${ANDROID_ABI}.so
    ${Qt6_DIR}/../../libQt6Widgets_${ANDROID_ABI}.so
    ${Qt6_DIR}/../../libQt6Xml_${ANDROID_ABI}.so
    ${Qt6_DIR}/../../libQt6Test_${ANDROID_ABI}.so
    ${Qt6_DIR}/../../libQt6Svg_${ANDROID_ABI}.so
     ## --- END PILE OF SHAME --- ##

    ${OPENSSL_LIBS_DIR}/libcrypto.so
    ${OPENSSL_LIBS_DIR}/libssl.so
    ${OPENSSL_LIBS_DIR}/libcrypto_1_1.so
    ${OPENSSL_LIBS_DIR}/libssl_1_1.so
    ${OBFUSCATORS_LIB_LOCATION}
    APPEND)


if( ${Qt6_VERSION} VERSION_GREATER_EQUAL 6.4.0)
    set_property(TARGET mozillavpn PROPERTY QT_ANDROID_EXTRA_LIBS
        ${OPENSSL_LIBS_DIR}/libcrypto_3.so
        ${OPENSSL_LIBS_DIR}/libssl_3.so
    APPEND)
endif()

option(MZ_ANDROID_FOSS_BUILD "Build apk without google play services" OFF)

if(MZ_ANDROID_FOSS_BUILD)
    message(STATUS "Website build enabled")
    target_compile_definitions(mozillavpn PRIVATE "MZ_ANDROID_FOSS_BUILD=1")
endif()
