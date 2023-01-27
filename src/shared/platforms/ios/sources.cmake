# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/macos/macoscryptosettings.mm
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/ioscommons.h
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/ios/ioscommons.mm
)
