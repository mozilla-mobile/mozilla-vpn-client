# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This section integrates the OpenSSL-Prefab into our build system.  Google
# Publishes builds of openSSL for android on maven.
# However, that Google Maven one is out of date. So we use the KDAB one.

# include(ExternalProject)

# ExternalProject_Add(ndk_openssl
#   URL https://maven.google.com/com/android/ndk/thirdparty/openssl/1.1.1q-beta-1/openssl-1.1.1q-beta-1.aar
#   URL_HASH SHA256=a5b05c4b362d35c022238ef9b2e4e2196248adea3bac9dd683845ee75a3a8d66
#   BUILD_IN_SOURCE 1
#   DOWNLOAD_NAME "openssl_android.zip" # Save it as zip, so cmake knows to extract.
#   CONFIGURE_COMMAND ""
#   BUILD_COMMAND ""
#   INSTALL_COMMAND ""
# )

# Sadly, since we're not building the client in the gradle enviroment
# We can't use this "raw". 
# These 2 Tasks fetch the artifact and unpack them, making it usable for us.

# It will Set 3 Global Properties to read from 
# OPENSSL_CRYPTO_MODULE -> the prefab path of ssl::crypto (abi-checked)
# OPENSSL_SSL_MODULE -> the prefab path of ssl::ssl (abi-checked)
# OPENSSL_LIBS -> a folder containing both shared librarys.
# Note: OPENSSL_LIBS has each file 2 times:
# once named <libname>.so and <libname>_1_1.so
# QT only seeems to load them if they are called x.1_1.so
# For the Rust/Openssl-sys crate we need x.so

# Copy them in one folder so that they can be used in 
# rust-openssl's env:OPENSSL_LIB_DIR
# and qt's extra libs. 
add_custom_target(ndk_openssl_merged)

# In case of newer versions of QT, for QtSLL we need to 
# have 3.x.x bundled alongside.
# Some rust crates seem to need 1.x? so ugh. both i guess. 
# This piece just add's the _3.so libs into the merged folder.
find_package(Qt6 COMPONENTS Core)
include(FetchContent)

  # Google does not provide openssl v 3.0
  # so let's use the builds from kdab
  FetchContent_Declare(
    android_openssl
    DOWNLOAD_EXTRACT_TIMESTAMP true
    GIT_REPOSITORY https://github.com/KDAB/android_openssl.git
    GIT_TAG 32ebe304ff064a9affb699b2185af78e3494f49a # Latest commit as of July 2nd, 2025 # UPDATE DATE HERE
  )
  FetchContent_MakeAvailable(android_openssl)

SET( _OPENSSL_MODULE "${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/")
SET( _OPENSSL_LIBS "${android_openssl_SOURCE_DIR}/combined_libs") 
file(MAKE_DIRECTORY ${_OPENSSL_LIBS})


set_property(GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE ${_OPENSSL_MODULE})
set_property(GLOBAL PROPERTY OPENSSL_SSL_MODULE ${_OPENSSL_MODULE})
set_property(GLOBAL PROPERTY OPENSSL_LIBS ${_OPENSSL_LIBS})

  add_custom_command(
    TARGET ndk_openssl_merged
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_3/${ANDROID_ABI}/libssl_3.so ${_OPENSSL_LIBS}/libssl_3.so
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_3/${ANDROID_ABI}/libcrypto_3.so ${_OPENSSL_LIBS}/libcrypto_3.so
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libssl.so ${_OPENSSL_LIBS}/libssl.so
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libcrypto.so ${_OPENSSL_LIBS}/libcrypto.so
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libssl_1_1.so ${_OPENSSL_LIBS}/libssl_1_1.so
    COMMAND ${CMAKE_COMMAND} -E copy ${android_openssl_SOURCE_DIR}/ssl_1.1/${ANDROID_ABI}/libcrypto_1_1.so ${_OPENSSL_LIBS}/libcrypto_1_1.so
  )

