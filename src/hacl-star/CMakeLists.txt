# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_hacl-star STATIC)

target_sources(mozillavpn_hacl-star INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Chacha20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Chacha20.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Chacha20Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Chacha20Poly1305_32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Curve25519_51.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Curve25519_51.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Kremlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Hacl_Poly1305_32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/lowstar_endianness.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/internal/callconv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/internal/compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/internal/target.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/internal/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/FStar_UInt128.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/FStar_UInt128_Verified.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/FStar_UInt_8_16_32_64.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/LowStar_Endianness.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/fstar_uint128_gcc64.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/fstar_uint128_msvc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal/fstar_uint128_struct_endianness.h
)

target_include_directories(mozillavpn_hacl-star PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# By making these public, any target which links to this target
# will add these directories to their include directories list.
target_include_directories(mozillavpn_hacl-star PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin
    ${CMAKE_CURRENT_SOURCE_DIR}/kremlin/minimal
)
