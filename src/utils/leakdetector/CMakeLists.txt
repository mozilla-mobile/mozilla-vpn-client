# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_utils_leakdetector
    SOURCES
        leakdetector.h
        leakdetector.cpp
    TEST_SOURCES
        tests/testleakdetector.cpp
    QT_DEPENDENCIES
        Core
)
