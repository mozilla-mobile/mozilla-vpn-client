# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(RUST_DEPENDENCIES)
if (${MZ_PLATFORM_NAME} STREQUAL "linux"
    OR ${MZ_PLATFORM_NAME} STREQUAL "windows"
    OR ${MZ_PLATFORM_NAME} STREQUAL "macos")
    list(APPEND RUST_DEPENDENCIES ${CMAKE_CURRENT_SOURCE_DIR}/signature/signature)
endif()

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
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/signature.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/signature.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/androidsignature.cpp
    IOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/iossignature.mm
    MACOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/rustsignature.cpp
    WINDOWS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/rustsignature.cpp
    LINUX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/rustsignature.cpp
    WASM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/dummysignature.cpp
    DUMMY_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/signature/dummysignature.cpp
    QT_DEPENDENCIES
        Core
        Network
    MZ_DEPENDENCIES
        mz_logging
    RUST_DEPENDENCIES
        ${RUST_DEPENDENCIES}
)
