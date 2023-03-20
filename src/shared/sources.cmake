# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(shared-sources INTERFACE)

if(NOT MSVC)
  target_compile_options(shared-sources INTERFACE -Wall -Werror -Wno-conversion)
endif()

# Generated version header file
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(shared-sources INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set_property(TARGET shared-sources PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Shared components
target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/app.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinapp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinapplistener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinapplistener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinappsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/authenticationinappsession.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/incrementaldecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationinapp/incrementaldecoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationlistener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/authenticationlistener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/collator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/collator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/cryptosettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/cryptosettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/curve25519.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/env.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/errorhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/errorhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/feature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/feature.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/filterproxymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/filterproxymodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/fontloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/fontloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigationbarbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigationbarbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigationbarmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigationbarmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigatorreloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/frontend/navigatorreloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/gleandeprecated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/gleandeprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/mzglean.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/glean/mzglean.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hawkauth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/hkdf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ipaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/itempicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/itempicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/languagei18n.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/languagei18n.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/leakdetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/localizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/localizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logoutobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logoutobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/loghandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/loghandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/models/licensemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/models/licensemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkrequest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/networkrequest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlengineholder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlpath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/qmlpath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1112.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1112.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1918.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc1918.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4193.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4193.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4291.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc4291.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc5735.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rfc/rfc5735.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settingsholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settingsholder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/signature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/signature.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/simplenetworkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/simplenetworkmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/task.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/taskscheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/taskscheduler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/authenticate/taskauthenticate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/authenticate/taskauthenticate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/deleteaccount/taskdeleteaccount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/deleteaccount/taskdeleteaccount.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/function/taskfunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/function/taskfunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/group/taskgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/group/taskgroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/temporarydir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/temporarydir.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/theme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/theme.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/urlopener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/urlopener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/versionutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/versionutils.h
)

target_sources(shared-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/resources/license.qrc
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/signalhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/signalhandler.h
     )
endif()

# Sources for desktop platforms.
if(NOT CMAKE_CROSSCOMPILING)
     target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/authenticate/desktopauthenticationlistener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/tasks/authenticate/desktopauthenticationlistener.h
       )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR
   ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR
   ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Compile and link the signature library.
    include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)
    add_rust_library(signature
        PACKAGE_DIR ${CMAKE_SOURCE_DIR}/signature
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
        CRATE_NAME signature
    )
    target_compile_definitions(shared-sources INTERFACE MZ_SIGNATURE)
    target_link_libraries(shared-sources INTERFACE signature)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/${MZ_PLATFORM_NAME}/sources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/shared/cmake/sentry.cmake)
