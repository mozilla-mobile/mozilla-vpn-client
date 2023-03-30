# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SHARED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/shared ABSOLUTE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libsecret REQUIRED libsecret-1)
target_include_directories(shared-sources INTERFACE ${libsecret_INCLUDE_DIRS})

target_sources(shared-sources INTERFACE
    ${MZ_SHARED_SOURCE_DIR}/eventlistener.cpp
    ${MZ_SHARED_SOURCE_DIR}/eventlistener.h
    ${MZ_SHARED_SOURCE_DIR}/platforms/linux/linuxcryptosettings.cpp
)
