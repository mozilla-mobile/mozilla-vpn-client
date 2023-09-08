# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/cmake/generate_translations_target.cmake)

generate_translations_target(translations ${CMAKE_SOURCE_DIR}/src/translations)
