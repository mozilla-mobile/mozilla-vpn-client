# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This section integrates the OpenSSL-Prefab into our build system.  Google
# Publishes builds of openSSL for android on maven.
# However, that Google Maven one is out of date. So we use the KDAB one.

# It will set 3 Global Properties to read from 
# OPENSSL_SSL_MODULE -> the prefab path of ssl::ssl (abi-checked)
# OPENSSL_LIBS -> a folder containing all shared libraries.
# Note: OPENSSL_LIBS has each file 3 times:
# once named <libname>.so and <libname>_1_1.so and <libname>_3.so
# QT only seeems to load them if they are called x.1_1.so
# For the Rust/Openssl-sys crate we need x.so
# For newer versions of QT (6.4+), we need 3.x.x bundled alongside.

add_custom_target(ndk_openssl_merged)

find_package(Qt6 COMPONENTS Core)
include(FetchContent)

FetchContent_Declare(
  android_openssl
  DOWNLOAD_EXTRACT_TIMESTAMP true
  GIT_REPOSITORY https://github.com/KDAB/android_openssl.git
  GIT_TAG 32ebe304ff064a9affb699b2185af78e3494f49a # Latest commit as of November 17, 2025
)
FetchContent_MakeAvailable(android_openssl)

SET( _OPENSSL_MODULE "${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/")
SET( _OPENSSL_LIBS "${android_openssl_SOURCE_DIR}/combined_libs") 
file(MAKE_DIRECTORY ${_OPENSSL_LIBS})

set_property(GLOBAL PROPERTY OPENSSL_SSL_MODULE ${_OPENSSL_MODULE})
set_property(GLOBAL PROPERTY OPENSSL_LIBS ${_OPENSSL_LIBS})

# Copy them in one folder so that they can be used in
# rust-openssl's env:OPENSSL_LIB_DIR
# and qt's extra libs.
add_custom_command(
  TARGET ndk_openssl_merged
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_3/${ANDROID_ABI}/libssl_3.so ${_OPENSSL_LIBS}/libssl_3.so
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_3/${ANDROID_ABI}/libcrypto_3.so ${_OPENSSL_LIBS}/libcrypto_3.so
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libssl.so ${_OPENSSL_LIBS}/libssl.so
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libcrypto.so ${_OPENSSL_LIBS}/libcrypto.so
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libssl_1_1.so ${_OPENSSL_LIBS}/libssl_1_1.so
  COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libcrypto_1_1.so ${_OPENSSL_LIBS}/libcrypto_1_1.so
)
