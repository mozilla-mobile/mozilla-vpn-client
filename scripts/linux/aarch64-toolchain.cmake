# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CROSS_TRIPLE aarch64-linux-gnu)

# Cross-compiler executables — use full paths so rpmbuild's restricted PATH is not an issue.
# Both Debian and Fedora install cross-compilers under /usr/bin/.
set(CMAKE_C_COMPILER        /usr/bin/${CROSS_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER      /usr/bin/${CROSS_TRIPLE}-g++)
# Rust target triple (used by scripts/cmake/rustlang.cmake for cross-compilation)
set(CMAKE_C_COMPILER_TARGET aarch64-unknown-linux-gnu)
set(CMAKE_AR           /usr/bin/${CROSS_TRIPLE}-ar       CACHE FILEPATH "Archiver for aarch64")
set(CMAKE_RANLIB       /usr/bin/${CROSS_TRIPLE}-ranlib    CACHE FILEPATH "Ranlib for aarch64")
set(CMAKE_STRIP        /usr/bin/${CROSS_TRIPLE}-strip     CACHE FILEPATH "Strip for aarch64")
set(CMAKE_OBJCOPY      /usr/bin/${CROSS_TRIPLE}-objcopy   CACHE FILEPATH "Objcopy for aarch64")

set(CMAKE_FIND_ROOT_PATH
    /usr/${CROSS_TRIPLE}
    /usr/lib/${CROSS_TRIPLE}
)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# ${CMAKE_SYSROOT}/usr/lib64 (Fedora aarch64 native layout).
set(ENV{PKG_CONFIG_PATH}        /usr/lib/${CROSS_TRIPLE}/pkgconfig:/usr/share/pkgconfig)
set(ENV{PKG_CONFIG_LIBDIR}      /usr/lib/${CROSS_TRIPLE}/pkgconfig:/usr/share/pkgconfig)
set(ENV{PKG_CONFIG_SYSROOT_DIR} /)

