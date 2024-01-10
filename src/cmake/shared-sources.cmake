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

target_link_libraries(shared-sources INTERFACE
    mz_context
    mz_settings
    mz_logging
    mz_feature
    mz_utilities
    mz_crypto
)

# Shared components
target_sources(shared-sources INTERFACE
    ${CMAKE_SOURCE_DIR}/src/settingslist.h
    ${CMAKE_SOURCE_DIR}/src/addons/addon.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addon.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonapi.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonapi.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonguide.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonguide.h
    ${CMAKE_SOURCE_DIR}/src/addons/addoni18n.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addoni18n.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonmessage.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonmessage.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonproperty.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonproperty.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonpropertylist.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonpropertylist.h
    ${CMAKE_SOURCE_DIR}/src/addons/addonreplacer.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/addonreplacer.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcher.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchergroup.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertime.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addondirectory.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addondirectory.h
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addonindex.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addonindex.h
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addonmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/manager/addonmanager.h
    ${CMAKE_SOURCE_DIR}/src/addons/state/addonstate.h
    ${CMAKE_SOURCE_DIR}/src/addons/state/addonstatebase.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/state/addonstatebase.h
    ${CMAKE_SOURCE_DIR}/src/addons/state/addonsessionstate.cpp
    ${CMAKE_SOURCE_DIR}/src/addons/state/addonsessionstate.h
    ${CMAKE_SOURCE_DIR}/src/app.cpp
    ${CMAKE_SOURCE_DIR}/src/app.h
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
    ${CMAKE_SOURCE_DIR}/src/errorhandler.cpp
    ${CMAKE_SOURCE_DIR}/src/errorhandler.h
    ${CMAKE_SOURCE_DIR}/src/feature/feature.cpp
    ${CMAKE_SOURCE_DIR}/src/feature/feature.h
    ${CMAKE_SOURCE_DIR}/src/fontloader.cpp
    ${CMAKE_SOURCE_DIR}/src/fontloader.h
    ${CMAKE_SOURCE_DIR}/src/frontend/navigator.cpp
    ${CMAKE_SOURCE_DIR}/src/frontend/navigator.h
    ${CMAKE_SOURCE_DIR}/src/frontend/navigationbarbutton.cpp
    ${CMAKE_SOURCE_DIR}/src/frontend/navigationbarbutton.h
    ${CMAKE_SOURCE_DIR}/src/frontend/navigationbarmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/frontend/navigationbarmodel.h
    ${CMAKE_SOURCE_DIR}/src/frontend/navigatorreloader.cpp
    ${CMAKE_SOURCE_DIR}/src/frontend/navigatorreloader.h
    ${CMAKE_SOURCE_DIR}/src/glean/mzglean.h
    ${CMAKE_SOURCE_DIR}/src/glean/mzglean.cpp
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
    ${CMAKE_SOURCE_DIR}/src/languagei18n.cpp
    ${CMAKE_SOURCE_DIR}/src/languagei18n.h
    ${CMAKE_SOURCE_DIR}/src/localizer.cpp
    ${CMAKE_SOURCE_DIR}/src/localizer.h
    ${CMAKE_SOURCE_DIR}/src/logoutobserver.cpp
    ${CMAKE_SOURCE_DIR}/src/logoutobserver.h
    ${CMAKE_SOURCE_DIR}/src/models/licensemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/licensemodel.h
    ${CMAKE_SOURCE_DIR}/src/networkmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/networkmanager.h
    ${CMAKE_SOURCE_DIR}/src/networkrequest.cpp
    ${CMAKE_SOURCE_DIR}/src/networkrequest.h
    ${CMAKE_SOURCE_DIR}/src/qmlpath.cpp
    ${CMAKE_SOURCE_DIR}/src/qmlpath.h
    ${CMAKE_SOURCE_DIR}/src/resourceloader.cpp
    ${CMAKE_SOURCE_DIR}/src/resourceloader.h
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
    ${CMAKE_SOURCE_DIR}/src/settingsholder.cpp
    ${CMAKE_SOURCE_DIR}/src/settingsholder.h
    ${CMAKE_SOURCE_DIR}/src/signature.cpp
    ${CMAKE_SOURCE_DIR}/src/signature.h
    ${CMAKE_SOURCE_DIR}/src/simplenetworkmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/simplenetworkmanager.h
    ${CMAKE_SOURCE_DIR}/src/task.h
    ${CMAKE_SOURCE_DIR}/src/taskscheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/taskscheduler.h
    ${CMAKE_SOURCE_DIR}/src/tasks/addon/taskaddon.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/addon/taskaddon.h
    ${CMAKE_SOURCE_DIR}/src/tasks/addonindex/taskaddonindex.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/addonindex/taskaddonindex.h
    ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/taskauthenticate.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/authenticate/taskauthenticate.h
    ${CMAKE_SOURCE_DIR}/src/tasks/deleteaccount/taskdeleteaccount.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/deleteaccount/taskdeleteaccount.h
    ${CMAKE_SOURCE_DIR}/src/tasks/getfeaturelist/taskgetfeaturelist.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/getfeaturelist/taskgetfeaturelist.h
    ${CMAKE_SOURCE_DIR}/src/tasks/function/taskfunction.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/function/taskfunction.h
    ${CMAKE_SOURCE_DIR}/src/tasks/group/taskgroup.cpp
    ${CMAKE_SOURCE_DIR}/src/tasks/group/taskgroup.h
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

include(${CMAKE_SOURCE_DIR}/src/platforms/${MZ_PLATFORM_NAME}/sources.cmake)
include(${CMAKE_SOURCE_DIR}/src/cmake/sentry.cmake)
