# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Integrate Sentry with shared-sources
# This file should be included after shared-sources is defined
# The sentry library itself is built in 3rdparty/sentry.cmake

if(NOT BUILD_CRASHREPORTING)
    return()
endif()

# Check if the sentry target is available
if(NOT TARGET sentry)
    return()
endif()

target_compile_definitions(shared-sources INTERFACE SENTRY_ENABLED)
target_compile_definitions(shared-sources INTERFACE SENTRY_NONE_TRANSPORT)
target_compile_definitions(shared-sources INTERFACE SENTRY_BUILD_STATIC)

target_link_libraries(shared-sources INTERFACE Qt6::QmlPrivate)

target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/sentryadapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sentryadapter.h
    ${CMAKE_SOURCE_DIR}/src/tasks/sentry/tasksentry.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/sentry/tasksentry.h
    ${CMAKE_SOURCE_DIR}/src/tasks/sentryconfig/tasksentryconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/sentryconfig/tasksentryconfig.h
)

target_link_libraries(shared-sources INTERFACE sentry)

if(WIN32)
    target_link_libraries(shared-sources INTERFACE dbghelp.lib)
    target_link_libraries(shared-sources INTERFACE version.lib)
endif()
