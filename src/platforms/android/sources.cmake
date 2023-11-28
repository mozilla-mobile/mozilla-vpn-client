# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcryptosettings.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.h
)

if(ADJUST_TOKEN)
    message("Adjust SDK enabled")
    # SDK Token present, let's enable that.
    target_compile_definitions(mozillavpn PUBLIC MZ_ADJUST)

    target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustfiltering.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustfiltering.h
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjusthandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjusthandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxyconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxyconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxypackagehandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjustproxypackagehandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjusttasksubmission.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adjust/adjusttasksubmission.h
    )
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(${CMAKE_BUILD_TYPE})
        message( FATAL_ERROR "Adjust token cannot be empty for release builds")
    endif()
endif()

add_dependencies(shared-sources ndk_openssl_merged)

get_property(crypto_module GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE)
get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)

target_include_directories(shared-sources INTERFACE ${ssl_module}/include)

target_link_directories(shared-sources INTERFACE ${openssl_libs})
target_link_libraries(shared-sources INTERFACE libcrypto.so)
target_link_libraries(shared-sources INTERFACE libssl.so)
target_link_libraries(shared-sources INTERFACE -ljnigraphics)
