# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(mozillavpn-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosauthenticationlistener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosauthenticationlistener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscryptosettings.mm
)
