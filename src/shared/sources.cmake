# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(shared-sources INTERFACE)
get_filename_component(MZ_SHARED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/shared ABSOLUTE)

if(NOT MSVC AND NOT IOS)
  target_compile_options(shared-sources INTERFACE -Wall -Werror -Wno-conversion)
endif()

target_include_directories(shared-sources INTERFACE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MZ_SHARED_SOURCE_DIR}
    ${MZ_SHARED_SOURCE_DIR}/addons
    ${MZ_SHARED_SOURCE_DIR}/composer
    ${MZ_SHARED_SOURCE_DIR}/hacl-star
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/kremlin
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/kremlin/minimal
    ${MZ_SHARED_SOURCE_DIR}/glean
)

# Shared components
target_sources(shared-sources INTERFACE
    ${MZ_SHARED_SOURCE_DIR}/addons/addon.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addon.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addonapi.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addonapi.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addonguide.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addonguide.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addoni18n.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addoni18n.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addonmessage.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addonmessage.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addonproperty.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addonproperty.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addonpropertylist.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addonpropertylist.h
    ${MZ_SHARED_SOURCE_DIR}/addons/addontutorial.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/addontutorial.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcher.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcher.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchergroup.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertime.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addondirectory.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addondirectory.h
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addonindex.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addonindex.h
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addonmanager.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/manager/addonmanager.h
    ${MZ_SHARED_SOURCE_DIR}/addons/state/addonstate.h
    ${MZ_SHARED_SOURCE_DIR}/addons/state/addonstatebase.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/state/addonstatebase.h
    ${MZ_SHARED_SOURCE_DIR}/addons/state/addonsessionstate.cpp
    ${MZ_SHARED_SOURCE_DIR}/addons/state/addonsessionstate.h
    ${MZ_SHARED_SOURCE_DIR}/app.cpp
    ${MZ_SHARED_SOURCE_DIR}/app.h
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinapp.cpp
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinapp.h
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.cpp
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.h
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinappsession.cpp
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/authenticationinappsession.h
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/incrementaldecoder.cpp
    ${MZ_SHARED_SOURCE_DIR}/authenticationinapp/incrementaldecoder.h
    ${MZ_SHARED_SOURCE_DIR}/authenticationlistener.cpp
    ${MZ_SHARED_SOURCE_DIR}/authenticationlistener.h
    ${MZ_SHARED_SOURCE_DIR}/collator.cpp
    ${MZ_SHARED_SOURCE_DIR}/collator.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composer.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composer.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblock.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblock.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockbutton.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockbutton.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockorderedlist.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockorderedlist.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblocktext.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblocktext.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblocktitle.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblocktitle.h
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockunorderedlist.cpp
    ${MZ_SHARED_SOURCE_DIR}/composer/composerblockunorderedlist.h
    ${MZ_SHARED_SOURCE_DIR}/constants.cpp
    ${MZ_SHARED_SOURCE_DIR}/constants.h
    ${MZ_SHARED_SOURCE_DIR}/cryptosettings.cpp
    ${MZ_SHARED_SOURCE_DIR}/cryptosettings.h
    ${MZ_SHARED_SOURCE_DIR}/curve25519.cpp
    ${MZ_SHARED_SOURCE_DIR}/curve25519.h
    ${MZ_SHARED_SOURCE_DIR}/env.cpp
    ${MZ_SHARED_SOURCE_DIR}/env.h
    ${MZ_SHARED_SOURCE_DIR}/errorhandler.cpp
    ${MZ_SHARED_SOURCE_DIR}/errorhandler.h
    ${MZ_SHARED_SOURCE_DIR}/externalophandler.cpp
    ${MZ_SHARED_SOURCE_DIR}/externalophandler.h
    ${MZ_SHARED_SOURCE_DIR}/feature.cpp
    ${MZ_SHARED_SOURCE_DIR}/feature.h
    ${MZ_SHARED_SOURCE_DIR}/filterproxymodel.cpp
    ${MZ_SHARED_SOURCE_DIR}/filterproxymodel.h
    ${MZ_SHARED_SOURCE_DIR}/fontloader.cpp
    ${MZ_SHARED_SOURCE_DIR}/fontloader.h
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigator.cpp
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigator.h
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigationbarbutton.cpp
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigationbarbutton.h
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigationbarmodel.cpp
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigationbarmodel.h
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigatorreloader.cpp
    ${MZ_SHARED_SOURCE_DIR}/frontend/navigatorreloader.h
    ${MZ_SHARED_SOURCE_DIR}/glean/gleandeprecated.cpp
    ${MZ_SHARED_SOURCE_DIR}/glean/gleandeprecated.h
    ${MZ_SHARED_SOURCE_DIR}/glean/mzglean.h
    ${MZ_SHARED_SOURCE_DIR}/glean/mzglean.cpp
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/Hacl_Chacha20.c
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/Hacl_Curve25519_51.c
    ${MZ_SHARED_SOURCE_DIR}/hacl-star/Hacl_Poly1305_32.c
    ${MZ_SHARED_SOURCE_DIR}/hawkauth.cpp
    ${MZ_SHARED_SOURCE_DIR}/hawkauth.h
    ${MZ_SHARED_SOURCE_DIR}/hkdf.cpp
    ${MZ_SHARED_SOURCE_DIR}/hkdf.h
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorhandler.cpp
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorhandler.h
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectoritempicker.cpp
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectoritempicker.h
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorutils.cpp
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorutils.h
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorwebsocketconnection.cpp
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorwebsocketconnection.h
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorwebsocketserver.cpp
    ${MZ_SHARED_SOURCE_DIR}/inspector/inspectorwebsocketserver.h
    ${MZ_SHARED_SOURCE_DIR}/ipaddress.cpp
    ${MZ_SHARED_SOURCE_DIR}/ipaddress.h
    ${MZ_SHARED_SOURCE_DIR}/itempicker.cpp
    ${MZ_SHARED_SOURCE_DIR}/itempicker.h
    ${MZ_SHARED_SOURCE_DIR}/languagei18n.cpp
    ${MZ_SHARED_SOURCE_DIR}/languagei18n.h
    ${MZ_SHARED_SOURCE_DIR}/leakdetector.cpp
    ${MZ_SHARED_SOURCE_DIR}/leakdetector.h
    ${MZ_SHARED_SOURCE_DIR}/localizer.cpp
    ${MZ_SHARED_SOURCE_DIR}/localizer.h
    ${MZ_SHARED_SOURCE_DIR}/logoutobserver.cpp
    ${MZ_SHARED_SOURCE_DIR}/logoutobserver.h
    ${MZ_SHARED_SOURCE_DIR}/logger.cpp
    ${MZ_SHARED_SOURCE_DIR}/logger.h
    ${MZ_SHARED_SOURCE_DIR}/loghandler.cpp
    ${MZ_SHARED_SOURCE_DIR}/loghandler.h
    ${MZ_SHARED_SOURCE_DIR}/models/featuremodel.cpp
    ${MZ_SHARED_SOURCE_DIR}/models/featuremodel.h
    ${MZ_SHARED_SOURCE_DIR}/models/licensemodel.cpp
    ${MZ_SHARED_SOURCE_DIR}/models/licensemodel.h
    ${MZ_SHARED_SOURCE_DIR}/networkmanager.cpp
    ${MZ_SHARED_SOURCE_DIR}/networkmanager.h
    ${MZ_SHARED_SOURCE_DIR}/networkrequest.cpp
    ${MZ_SHARED_SOURCE_DIR}/networkrequest.h
    ${MZ_SHARED_SOURCE_DIR}/qmlengineholder.cpp
    ${MZ_SHARED_SOURCE_DIR}/qmlengineholder.h
    ${MZ_SHARED_SOURCE_DIR}/qmlpath.cpp
    ${MZ_SHARED_SOURCE_DIR}/qmlpath.h
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc1112.cpp
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc1112.h
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc1918.cpp
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc1918.h
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc4193.cpp
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc4193.h
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc4291.cpp
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc4291.h
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc5735.cpp
    ${MZ_SHARED_SOURCE_DIR}/rfc/rfc5735.h
    ${MZ_SHARED_SOURCE_DIR}/settingsholder.cpp
    ${MZ_SHARED_SOURCE_DIR}/settingsholder.h
    ${MZ_SHARED_SOURCE_DIR}/signature.cpp
    ${MZ_SHARED_SOURCE_DIR}/signature.h
    ${MZ_SHARED_SOURCE_DIR}/simplenetworkmanager.cpp
    ${MZ_SHARED_SOURCE_DIR}/simplenetworkmanager.h
    ${MZ_SHARED_SOURCE_DIR}/task.h
    ${MZ_SHARED_SOURCE_DIR}/taskscheduler.cpp
    ${MZ_SHARED_SOURCE_DIR}/taskscheduler.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/addon/taskaddon.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/addon/taskaddon.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/addonindex/taskaddonindex.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/addonindex/taskaddonindex.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/authenticate/taskauthenticate.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/authenticate/taskauthenticate.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/deleteaccount/taskdeleteaccount.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/deleteaccount/taskdeleteaccount.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/getfeaturelist/taskgetfeaturelist.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/getfeaturelist/taskgetfeaturelist.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/function/taskfunction.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/function/taskfunction.h
    ${MZ_SHARED_SOURCE_DIR}/tasks/group/taskgroup.cpp
    ${MZ_SHARED_SOURCE_DIR}/tasks/group/taskgroup.h
    ${MZ_SHARED_SOURCE_DIR}/temporarydir.cpp
    ${MZ_SHARED_SOURCE_DIR}/temporarydir.h
    ${MZ_SHARED_SOURCE_DIR}/theme.cpp
    ${MZ_SHARED_SOURCE_DIR}/theme.h
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorial.cpp
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorial.h
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstep.cpp
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstep.h
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstepbefore.cpp
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstepbefore.h
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstepnext.cpp
    ${MZ_SHARED_SOURCE_DIR}/tutorial/tutorialstepnext.h
    ${MZ_SHARED_SOURCE_DIR}/urlopener.cpp
    ${MZ_SHARED_SOURCE_DIR}/urlopener.h
    ${MZ_SHARED_SOURCE_DIR}/utils.cpp
    ${MZ_SHARED_SOURCE_DIR}/utils.h
    ${MZ_SHARED_SOURCE_DIR}/versionutils.cpp
    ${MZ_SHARED_SOURCE_DIR}/versionutils.h
    ${MZ_SHARED_SOURCE_DIR}/websocket/exponentialbackoffstrategy.cpp
    ${MZ_SHARED_SOURCE_DIR}/websocket/exponentialbackoffstrategy.h
    ${MZ_SHARED_SOURCE_DIR}/websocket/pushmessage.cpp
    ${MZ_SHARED_SOURCE_DIR}/websocket/pushmessage.h
    ${MZ_SHARED_SOURCE_DIR}/websocket/websockethandler.cpp
    ${MZ_SHARED_SOURCE_DIR}/websocket/websockethandler.h
)

target_sources(shared-sources INTERFACE
    ${MZ_SHARED_SOURCE_DIR}/resources/license.qrc
    ${MZ_SHARED_SOURCE_DIR}/resources/resources.qrc
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(shared-sources INTERFACE
        ${MZ_SHARED_SOURCE_DIR}/signalhandler.cpp
        ${MZ_SHARED_SOURCE_DIR}/signalhandler.h
     )
endif()

# Sources for desktop platforms.
if(NOT CMAKE_CROSSCOMPILING)
     target_sources(shared-sources INTERFACE
        ${MZ_SHARED_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.cpp
        ${MZ_SHARED_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.h
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

include(${MZ_SHARED_SOURCE_DIR}/platforms/${MZ_PLATFORM_NAME}/sources.cmake)
include(${MZ_SHARED_SOURCE_DIR}/cmake/sentry.cmake)
