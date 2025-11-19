# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Install the extension manifest
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    include(GNUInstallDirs)
    if(NOT DEFINED WEBEXT_INSTALL_LIBDIR)
        set(WEBEXT_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
    endif()

    configure_file(manifests/linux/mozillavpn.json.in manifests/linux/mozillavpn.json)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION ${WEBEXT_INSTALL_LIBDIR}/mozilla/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION /etc/opt/chrome/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION /etc/chromium/native-messaging-hosts)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/manifests/windows/mozillavpn.json
        DESTINATION .)
endif()

add_subdirectory(socks5proxy)
