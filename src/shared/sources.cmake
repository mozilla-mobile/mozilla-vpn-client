# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(shared-sources INTERFACE)

# Generated version header file
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(shared-sources INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set_property(TARGET shared-sources PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Shared components
target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/cryptosettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/cryptosettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/env.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/feature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/feature.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/filterproxymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/filterproxymodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/fontloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/fontloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/gleandeprecated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/gleandeprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/itempicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/itempicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/loghandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/loghandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlengineholder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlpath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlpath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1112.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1112.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1918.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1918.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4193.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4193.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4291.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4291.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc5735.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc5735.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settingsholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settingsholder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/simplenetworkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/simplenetworkmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/task.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/taskscheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/taskscheduler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/temporarydir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/temporarydir.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/urlopener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/urlopener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/versionutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/versionutils.h
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/signalhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/signalhandler.h
     )
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/${MZ_PLATFORM_NAME}/sources.cmake)
