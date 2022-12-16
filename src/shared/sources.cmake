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
    ${CMAKE_CURRENT_BINARY_DIR}
)

# VPN Client source files
target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
)
