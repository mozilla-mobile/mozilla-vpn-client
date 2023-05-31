# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set_property(TARGET mozillavpn APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../android/
)

target_link_libraries(mozillavpn PRIVATE
    Qt6::Test
    Qt6::Xml)

target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidiaphandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidnetworkwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidnotificationhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidvpnactivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidappimageprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidapplistprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidiaphandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidnetworkwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidnotificationhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidvpnactivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidappimageprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/android/androidapplistprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxpingsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxpingsender.h
)

get_property(OPENSSL_LIBS_DIR GLOBAL PROPERTY OPENSSL_LIBS)
get_property(QTGLEAN_LIB_LOCATION TARGET qtglean_bindings PROPERTY LOCATION_${CMAKE_BUILD_TYPE})

# This property flags the build system to copy these
# shared libraries into the expected Android shared library folder.
#
# Qt requires this to be set on the "app" target.
set_property(TARGET mozillavpn PROPERTY QT_ANDROID_EXTRA_LIBS
    ${OPENSSL_LIBS_DIR}/libcrypto.so
    ${OPENSSL_LIBS_DIR}/libssl.so
    ${QTGLEAN_LIB_LOCATION}
    APPEND)
