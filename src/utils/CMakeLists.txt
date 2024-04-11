# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mz_utils INTERFACE)

add_subdirectory(leakdetector)

target_link_libraries(mz_utils INTERFACE mz_utils_leakdetector)
