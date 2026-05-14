# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcryptosettings.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcryptosettings.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.h
)

target_link_libraries(shared-sources INTERFACE -ljnigraphics)
