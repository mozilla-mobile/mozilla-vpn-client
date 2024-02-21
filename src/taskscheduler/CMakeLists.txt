# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_taskscheduler
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/taskfunction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/taskfunction.h
        ${CMAKE_CURRENT_SOURCE_DIR}/taskgroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/taskgroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/task.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testtasks.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testtasks.h
    QT_DEPENDENCIES
        Core
    MZ_DEPENDENCIES
        mz_logging
        mz_utilities
)
