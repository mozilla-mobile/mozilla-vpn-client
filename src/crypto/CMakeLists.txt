# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_crypto
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Chacha20.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Chacha20.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Chacha20Poly1305_32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Chacha20Poly1305_32.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Curve25519_51.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Curve25519_51.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Kremlib.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Poly1305_32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/Hacl_Poly1305_32.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/lowstar_endianness.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/internal/callconv.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/internal/compat.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/internal/target.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/internal/types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/FStar_UInt128.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/FStar_UInt128_Verified.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/FStar_UInt_8_16_32_64.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/LowStar_Endianness.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/fstar_uint128_gcc64.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/fstar_uint128_msvc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal/fstar_uint128_struct_endianness.h
        ${CMAKE_CURRENT_SOURCE_DIR}/curve25519.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/curve25519.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hawkauth.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hawkauth.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hkdf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hkdf.h
    QT_DEPENDENCIES
        Core
        Network
)
