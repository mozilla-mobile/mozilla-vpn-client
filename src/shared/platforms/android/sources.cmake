# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/android/androidcommons.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/android/androidcommons.h
)

if(ADJUST_TOKEN)
    message(Adjust SDK enabled)
    # SDK Token present, let's enable that.
    add_compile_definitions("MZ_ADJUST")
    target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.h
    )
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(${CMAKE_BUILD_TYPE})
        message( FATAL_ERROR "Adjust token cannot be empty for release builds")
    endif()
endif()

# This section integrates the OpenSSL-Prefab into our build system.  Google
# Publishes builds of openSSL for android on maven. 

include(ExternalProject)
ExternalProject_Add(ndk_openssl
  URL https://maven.google.com/com/android/ndk/thirdparty/openssl/1.1.1q-beta-1/openssl-1.1.1q-beta-1.aar
  URL_HASH SHA256=a5b05c4b362d35c022238ef9b2e4e2196248adea3bac9dd683845ee75a3a8d66
  BUILD_IN_SOURCE 1
  DOWNLOAD_NAME "openssl_android.zip" # Save it as zip, so cmake knows to extract.
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

# Sadly, since we're not building the client in the gradle enviroment
# We can't use this "raw". 
# These 2 Tasks fetch the artifact and unpack them, making it usable for us.

# It will Set 3 Global Properties to read from 
# OPENSSL_CRYPTO_MODULE -> the prefab path of ssl::crypto (abi-checked)
# OPENSSL_SSL_MODULE -> the prefab path of ssl::ssl (abi-checked)
# OPENSSL_LIBS -> a folder containing both shared libarys.
# Note: OPENSSL_LIBS has each file 2 times:
# once named <libname>.so and <libname>_1_1.so
# QT only seeems to load them if they are called x.1_1.so
# For the Rust/Openssl-sys crate we need x.so

SET( _OPENSSL_CRYPTO_MODULE "${CMAKE_BINARY_DIR}/ndk_openssl-prefix/src/ndk_openssl/prefab/modules/crypto")
SET( _OPENSSL_SSL_MODULE "${CMAKE_BINARY_DIR}/ndk_openssl-prefix/src/ndk_openssl/prefab/modules/ssl")
SET( _OPENSSL_LIBS "${CMAKE_BINARY_DIR}/ndk_openssl/libs") 
SET( _OPENSSL_INCLUDE "${CMAKE_BINARY_DIR}/ndk_openssl/include") 

file(MAKE_DIRECTORY ${_OPENSSL_LIBS})
file(MAKE_DIRECTORY ${_OPENSSL_INCLUDE})

set_property(GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE ${_OPENSSL_CRYPTO_MODULE})
set_property(GLOBAL PROPERTY OPENSSL_SSL_MODULE ${_OPENSSL_SSL_MODULE})
set_property(GLOBAL PROPERTY OPENSSL_LIBS ${_OPENSSL_LIBS}) 

# Copy them in one folder so that they can be used in 
# rust-openssl's env:OPENSSL_LIB_DIR
# and qt's extra libs. 
add_custom_target(ndk_openssl_merged)
add_dependencies(ndk_openssl_merged ndk_openssl)
add_custom_command(
        TARGET ndk_openssl_merged
        COMMAND ${CMAKE_COMMAND} -E copy ${_OPENSSL_SSL_MODULE}/libs/android.${ANDROID_ABI}/libssl.so ${_OPENSSL_LIBS}/libssl_1_1.so
        COMMAND ${CMAKE_COMMAND} -E copy ${_OPENSSL_CRYPTO_MODULE}/libs/android.${ANDROID_ABI}/libcrypto.so ${_OPENSSL_LIBS}/libcrypto_1_1.so
        COMMAND ${CMAKE_COMMAND} -E copy ${_OPENSSL_SSL_MODULE}/libs/android.${ANDROID_ABI}/libssl.so ${_OPENSSL_LIBS}/libssl.so
        COMMAND ${CMAKE_COMMAND} -E copy ${_OPENSSL_CRYPTO_MODULE}/libs/android.${ANDROID_ABI}/libcrypto.so ${_OPENSSL_LIBS}/libcrypto.so)

add_dependencies(shared-sources ndk_openssl_merged)

get_property(crypto_module GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE)
get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)

target_include_directories(shared-sources INTERFACE ${ssl_module}/include)

get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)
set_property(TARGET shared-sources PROPERTY QT_ANDROID_EXTRA_LIBS
    ${openssl_libs}/libcrypto_1_1.so
    ${openssl_libs}/libssl_1_1.so)

target_link_directories(shared-sources INTERFACE ${openssl_libs})
target_link_libraries(shared-sources INTERFACE libcrypto.so)
target_link_libraries(shared-sources INTERFACE libssl.so)
target_link_libraries(shared-sources INTERFACE -ljnigraphics)
