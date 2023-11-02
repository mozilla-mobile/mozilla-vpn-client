# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(mozillavpn PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/dummy/dummycontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/dummy/dummycontroller.h
)
