# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

find_package(PkgConfig REQUIRED)
pkg_check_modules(libsecret REQUIRED libsecret-1)
target_include_directories(shared-sources INTERFACE ${libsecret_INCLUDE_DIRS})

target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/eventlistener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/eventlistener.h
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/linux/linuxcryptosettings.cpp
)
