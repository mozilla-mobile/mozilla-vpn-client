# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_dependencies(mozillavpn ndk_openssl_merged)

get_property(crypto_module GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE)
get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)


set_property(TARGET mozillavpn APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../android/
)

target_link_libraries(mozillavpn PRIVATE
    Qt6::Test
    Qt6::Xml)


target_link_libraries(
    mozillavpn PRIVATE
    -ljnigraphics)

target_sources(mozillavpn PRIVATE
    modules/vpn/platforms/linux/linuxpingsender.cpp
    modules/vpn/platforms/linux/linuxpingsender.h
    modules/vpn/platforms/android/androidappimageprovider.cpp
    modules/vpn/platforms/android/androidappimageprovider.h
    modules/vpn/platforms/android/androidcontroller.cpp
    modules/vpn/platforms/android/androidapplistprovider.cpp
    modules/vpn/platforms/android/androidapplistprovider.h
    modules/vpn/platforms/android/androidcontroller.h
    modules/vpn/platforms/android/androidnetworkwatcher.cpp
    modules/vpn/platforms/android/androidnetworkwatcher.h
    modules/vpn/platforms/android/androidvpnactivity.cpp
    modules/vpn/platforms/android/androidvpnactivity.h
    platforms/android/androidglean.cpp
    platforms/android/androidglean.h
    platforms/android/androidiaphandler.cpp
    platforms/android/androidiaphandler.h
    platforms/android/androidnotificationhandler.cpp
    platforms/android/androidnotificationhandler.h
    platforms/android/androidutils.cpp
    platforms/android/androidutils.h
    platforms/linux/linuxcryptosettings.cpp
    tasks/purchase/taskpurchase.cpp
    tasks/purchase/taskpurchase.h
)

if(ADJUST_TOKEN)
    message(Adjust SDK enabled)
    # SDK Token present, let's enable that.
    add_compile_definitions("MVPN_ADJUST")
    target_sources(mozillavpn PRIVATE
        adjust/adjustfiltering.cpp
        adjust/adjusthandler.cpp
        adjust/adjustproxy.cpp
        adjust/adjustproxyconnection.cpp
        adjust/adjustproxypackagehandler.cpp
        adjust/adjusttasksubmission.cpp
        adjust/adjustfiltering.h
        adjust/adjusthandler.h
        adjust/adjustproxy.h
        adjust/adjustproxyconnection.h
        adjust/adjustproxypackagehandler.h
        adjust/adjusttasksubmission.h
    )
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(${CMAKE_BUILD_TYPE})
        message( FATAL_ERROR "Adjust token cannot be empty for release builds")
    endif()
endif()


target_include_directories(mozillavpn PUBLIC ${ssl_module}/include)


get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)
set_property(TARGET mozillavpn PROPERTY QT_ANDROID_EXTRA_LIBS
    ${openssl_libs}/libcrypto_1_1.so
    ${openssl_libs}/libssl_1_1.so)

target_link_directories( mozillavpn PUBLIC ${openssl_libs})
target_link_libraries(mozillavpn PRIVATE libcrypto.so)
target_link_libraries(mozillavpn PRIVATE libssl.so)
