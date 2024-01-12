# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Library of sources shared between app and tests
add_library(shared-sources INTERFACE)

mz_target_handle_warnings(shared-sources)

set_property(TARGET shared-sources PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/glean
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Modules
add_subdirectory(${CMAKE_SOURCE_DIR}/src/settings)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/logging)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/feature)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/context)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/crypto)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/utilities)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/networking)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/taskscheduler)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/telemetry)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/addons)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/translations)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/navigator)

target_link_libraries(shared-sources INTERFACE
    mz_context
    mz_settings
    mz_logging
    mz_feature
    mz_utilities
    mz_crypto
    mz_networking
    mz_taskscheduler
    mz_telemetry
    mz_addons
    mz_translations
    mz_navigator
)

# Shared components
target_sources(shared-sources INTERFACE
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinapp.cpp
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinapp.h
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinapplistener.cpp
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinapplistener.h
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinappsession.cpp
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/authenticationinappsession.h
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/incrementaldecoder.cpp
    ${CMAKE_SOURCE_DIR}/src/authenticationinapp/incrementaldecoder.h
    ${CMAKE_SOURCE_DIR}/src/authenticationlistener.cpp
    ${CMAKE_SOURCE_DIR}/src/authenticationlistener.h
    ${CMAKE_SOURCE_DIR}/src/collator.cpp
    ${CMAKE_SOURCE_DIR}/src/collator.h
    ${CMAKE_SOURCE_DIR}/src/feature/feature.cpp
    ${CMAKE_SOURCE_DIR}/src/feature/feature.h
    ${CMAKE_SOURCE_DIR}/src/fontloader.cpp
    ${CMAKE_SOURCE_DIR}/src/fontloader.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorhandler.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorhandler.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorhotreloader.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorhotreloader.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectoritempicker.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectoritempicker.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorutils.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorutils.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorwebsocketconnection.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorwebsocketconnection.h
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorwebsocketserver.cpp
    ${CMAKE_SOURCE_DIR}/src/inspector/inspectorwebsocketserver.h
    ${CMAKE_SOURCE_DIR}/src/ipaddress.cpp
    ${CMAKE_SOURCE_DIR}/src/ipaddress.h
    ${CMAKE_SOURCE_DIR}/src/itempicker.cpp
    ${CMAKE_SOURCE_DIR}/src/itempicker.h
    ${CMAKE_SOURCE_DIR}/src/logoutobserver.cpp
    ${CMAKE_SOURCE_DIR}/src/logoutobserver.h
    ${CMAKE_SOURCE_DIR}/src/models/licensemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/licensemodel.h
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc1112.cpp
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc1112.h
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc1918.cpp
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc1918.h
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc4193.cpp
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc4193.h
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc4291.cpp
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc4291.h
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc5735.cpp
    ${CMAKE_SOURCE_DIR}/src/rfc/rfc5735.h
    ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/taskauthenticate.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/taskauthenticate.h
    ${CMAKE_SOURCE_DIR}/src/tasks/deleteaccount/taskdeleteaccount.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/deleteaccount/taskdeleteaccount.h
    ${CMAKE_SOURCE_DIR}/src/tasks/getfeaturelist/taskgetfeaturelist.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/getfeaturelist/taskgetfeaturelist.h
    ${CMAKE_SOURCE_DIR}/src/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/theme.h
)

target_sources(shared-sources INTERFACE
    ${CMAKE_SOURCE_DIR}/src/resources/license.qrc
    ${CMAKE_SOURCE_DIR}/src/resources/resources.qrc
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(shared-sources INTERFACE
        ${CMAKE_SOURCE_DIR}/src/signalhandler.cpp
        ${CMAKE_SOURCE_DIR}/src/signalhandler.h
     )
endif()

# Sources for desktop platforms.
if(NOT CMAKE_CROSSCOMPILING)
     target_sources(shared-sources INTERFACE
        ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/desktopauthenticationlistener.cpp
        ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/desktopauthenticationlistener.h
       )
endif()

include(${CMAKE_SOURCE_DIR}/src/platforms/${MZ_PLATFORM_NAME}/sources.cmake)
include(${CMAKE_SOURCE_DIR}/src/cmake/sentry.cmake)
