# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/apps/vpn)
include_directories(${MZ_SOURCE_DIR}/shared)
include_directories(${MZ_SOURCE_DIR}/shared/addons)
include_directories(${MZ_SOURCE_DIR}/shared/composer)
include_directories(${MZ_SOURCE_DIR}/shared/glean)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)
include_directories(${CMAKE_SOURCE_DIR}/glean)

qt_add_executable(qml_tests EXCLUDE_FROM_ALL)
set_target_properties(qml_tests PROPERTIES FOLDER "Tests")
add_dependencies(build_tests qml_tests)

target_link_libraries(qml_tests PRIVATE
    Qt6::Gui
    Qt6::NetworkAuth
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickTest
)

target_link_libraries(qml_tests PRIVATE glean qtglean lottie nebula translations_vpn)

target_compile_definitions(qml_tests PRIVATE QUICK_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
target_compile_definitions(qml_tests PRIVATE UNIT_TEST)
target_compile_definitions(qml_tests PRIVATE MZ_DUMMY)

# VPN Client source files
target_sources(qml_tests PRIVATE
    ${MZ_SOURCE_DIR}/apps/vpn/appconstants.h
    ${MZ_SOURCE_DIR}/apps/vpn/controller.h
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/location.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/location.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/server.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/server.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercity.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercity.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercountry.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercountry.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercountrymodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/servercountrymodel.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/serverdata.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/serverdata.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/subscriptiondata.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/subscriptiondata.h
    ${MZ_SOURCE_DIR}/apps/vpn/mozillavpn.h
    ${MZ_SOURCE_DIR}/apps/vpn/pinghelper.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/pinghelper.h
    ${MZ_SOURCE_DIR}/apps/vpn/pingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/pingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/pingsenderfactory.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/pingsenderfactory.h
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/serveri18n.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/serveri18n.h
    ${MZ_SOURCE_DIR}/apps/vpn/serverlatency.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/serverlatency.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/controlleraction/taskcontrolleraction.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/controlleraction/taskcontrolleraction.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/updater.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/updater.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/versionapi.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/versionapi.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/webupdater.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/webupdater.h
    ${MZ_SOURCE_DIR}/shared/addons/addon.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addon.h
    ${MZ_SOURCE_DIR}/shared/addons/addonapi.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonapi.h
    ${MZ_SOURCE_DIR}/shared/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonguide.h
    ${MZ_SOURCE_DIR}/shared/addons/addoni18n.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addoni18n.h
    ${MZ_SOURCE_DIR}/shared/addons/addonmessage.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonmessage.h
    ${MZ_SOURCE_DIR}/shared/addons/addonproperty.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonproperty.h
    ${MZ_SOURCE_DIR}/shared/addons/addonpropertylist.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonpropertylist.h
    ${MZ_SOURCE_DIR}/shared/addons/addontutorial.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addontutorial.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcher.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcher.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchergroup.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertime.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${MZ_SOURCE_DIR}/shared/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${MZ_SOURCE_DIR}/shared/addons/manager/addondirectory.cpp
    ${MZ_SOURCE_DIR}/shared/addons/manager/addondirectory.h
    ${MZ_SOURCE_DIR}/shared/addons/manager/addonindex.cpp
    ${MZ_SOURCE_DIR}/shared/addons/manager/addonindex.h
    ${MZ_SOURCE_DIR}/shared/addons/manager/addonmanager.cpp
    ${MZ_SOURCE_DIR}/shared/addons/manager/addonmanager.h
    ${MZ_SOURCE_DIR}/shared/addons/state/addonstate.h
    ${MZ_SOURCE_DIR}/shared/addons/state/addonstatebase.cpp
    ${MZ_SOURCE_DIR}/shared/addons/state/addonstatebase.h
    ${MZ_SOURCE_DIR}/shared/addons/state/addonsessionstate.cpp
    ${MZ_SOURCE_DIR}/shared/addons/state/addonsessionstate.h
    ${MZ_SOURCE_DIR}/shared/app.cpp
    ${MZ_SOURCE_DIR}/shared/app.h
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinapp.cpp
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinapp.h
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinapplistener.cpp
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinapplistener.h
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinappsession.cpp
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/authenticationinappsession.h
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/incrementaldecoder.cpp
    ${MZ_SOURCE_DIR}/shared/authenticationinapp/incrementaldecoder.h
    ${MZ_SOURCE_DIR}/shared/authenticationlistener.cpp
    ${MZ_SOURCE_DIR}/shared/authenticationlistener.h
    ${MZ_SOURCE_DIR}/shared/collator.cpp
    ${MZ_SOURCE_DIR}/shared/collator.h
    ${MZ_SOURCE_DIR}/shared/composer/composer.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composer.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblock.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblock.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblockbutton.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblockbutton.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblockorderedlist.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblockorderedlist.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblocktext.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblocktext.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblocktitle.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblocktitle.h
    ${MZ_SOURCE_DIR}/shared/composer/composerblockunorderedlist.cpp
    ${MZ_SOURCE_DIR}/shared/composer/composerblockunorderedlist.h
    ${MZ_SOURCE_DIR}/shared/constants.h
    ${MZ_SOURCE_DIR}/shared/cryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/cryptosettings.h
    ${MZ_SOURCE_DIR}/shared/curve25519.cpp
    ${MZ_SOURCE_DIR}/shared/curve25519.h
    ${MZ_SOURCE_DIR}/shared/env.cpp
    ${MZ_SOURCE_DIR}/shared/env.h
    ${MZ_SOURCE_DIR}/shared/errorhandler.cpp
    ${MZ_SOURCE_DIR}/shared/errorhandler.h
    ${MZ_SOURCE_DIR}/shared/externalophandler.cpp
    ${MZ_SOURCE_DIR}/shared/externalophandler.h
    ${MZ_SOURCE_DIR}/shared/feature.cpp
    ${MZ_SOURCE_DIR}/shared/feature.h
    ${MZ_SOURCE_DIR}/shared/filterproxymodel.cpp
    ${MZ_SOURCE_DIR}/shared/filterproxymodel.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigator.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigator.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarbutton.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarbutton.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarmodel.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarmodel.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigatorreloader.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigatorreloader.h
    ${MZ_SOURCE_DIR}/shared/glean/gleandeprecated.cpp
    ${MZ_SOURCE_DIR}/shared/glean/gleandeprecated.h
    ${MZ_SOURCE_DIR}/shared/glean/mzglean.h
    ${MZ_SOURCE_DIR}/shared/glean/mzglean.cpp
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
    ${MZ_SOURCE_DIR}/shared/hawkauth.cpp
    ${MZ_SOURCE_DIR}/shared/hawkauth.h
    ${MZ_SOURCE_DIR}/shared/hkdf.cpp
    ${MZ_SOURCE_DIR}/shared/hkdf.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorhandler.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorutils.cpp
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorutils.h
    ${MZ_SOURCE_DIR}/shared/ipaddress.cpp
    ${MZ_SOURCE_DIR}/shared/ipaddress.h
    ${MZ_SOURCE_DIR}/shared/itempicker.cpp
    ${MZ_SOURCE_DIR}/shared/itempicker.h
    ${MZ_SOURCE_DIR}/shared/languagei18n.cpp
    ${MZ_SOURCE_DIR}/shared/languagei18n.h
    ${MZ_SOURCE_DIR}/shared/leakdetector.cpp
    ${MZ_SOURCE_DIR}/shared/leakdetector.h
    ${MZ_SOURCE_DIR}/shared/localizer.cpp
    ${MZ_SOURCE_DIR}/shared/localizer.h
    ${MZ_SOURCE_DIR}/shared/logger.cpp
    ${MZ_SOURCE_DIR}/shared/logger.h
    ${MZ_SOURCE_DIR}/shared/loghandler.cpp
    ${MZ_SOURCE_DIR}/shared/loghandler.h
    ${MZ_SOURCE_DIR}/shared/models/featuremodel.cpp
    ${MZ_SOURCE_DIR}/shared/models/featuremodel.h
    ${MZ_SOURCE_DIR}/shared/models/licensemodel.cpp
    ${MZ_SOURCE_DIR}/shared/models/licensemodel.h
    ${MZ_SOURCE_DIR}/shared/models/supportcategorymodel.cpp
    ${MZ_SOURCE_DIR}/shared/models/supportcategorymodel.h
    ${MZ_SOURCE_DIR}/shared/networkmanager.cpp
    ${MZ_SOURCE_DIR}/shared/networkmanager.h
    ${MZ_SOURCE_DIR}/shared/networkrequest.cpp
    ${MZ_SOURCE_DIR}/shared/networkrequest.h
    ${MZ_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.h
    ${MZ_SOURCE_DIR}/shared/qmlpath.cpp
    ${MZ_SOURCE_DIR}/shared/qmlpath.h
    ${MZ_SOURCE_DIR}/shared/rfc/rfc1918.cpp
    ${MZ_SOURCE_DIR}/shared/rfc/rfc1918.h
    ${MZ_SOURCE_DIR}/shared/rfc/rfc4193.cpp
    ${MZ_SOURCE_DIR}/shared/rfc/rfc4193.h
    ${MZ_SOURCE_DIR}/shared/rfc/rfc4291.cpp
    ${MZ_SOURCE_DIR}/shared/rfc/rfc4291.h
    ${MZ_SOURCE_DIR}/shared/rfc/rfc5735.cpp
    ${MZ_SOURCE_DIR}/shared/rfc/rfc5735.h
    ${MZ_SOURCE_DIR}/shared/settingsholder.cpp
    ${MZ_SOURCE_DIR}/shared/settingsholder.h
    ${MZ_SOURCE_DIR}/shared/simplenetworkmanager.cpp
    ${MZ_SOURCE_DIR}/shared/simplenetworkmanager.h
    ${MZ_SOURCE_DIR}/shared/signature.cpp
    ${MZ_SOURCE_DIR}/shared/signature.h
    ${MZ_SOURCE_DIR}/shared/task.h
    ${MZ_SOURCE_DIR}/shared/taskscheduler.cpp
    ${MZ_SOURCE_DIR}/shared/taskscheduler.h
    ${MZ_SOURCE_DIR}/shared/tasks/addon/taskaddon.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/addon/taskaddon.h
    ${MZ_SOURCE_DIR}/shared/tasks/addonindex/taskaddonindex.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/addonindex/taskaddonindex.h
    ${MZ_SOURCE_DIR}/shared/tasks/authenticate/desktopauthenticationlistener.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/authenticate/desktopauthenticationlistener.h
    ${MZ_SOURCE_DIR}/shared/tasks/createsupportticket/taskcreatesupportticket.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/createsupportticket/taskcreatesupportticket.h
    ${MZ_SOURCE_DIR}/shared/tasks/function/taskfunction.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/function/taskfunction.h
    ${MZ_SOURCE_DIR}/shared/tasks/group/taskgroup.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/group/taskgroup.h
    ${MZ_SOURCE_DIR}/shared/temporarydir.cpp
    ${MZ_SOURCE_DIR}/shared/temporarydir.h
    ${MZ_SOURCE_DIR}/shared/theme.cpp
    ${MZ_SOURCE_DIR}/shared/theme.h
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorial.cpp
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorial.h
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstep.cpp
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstep.h
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstepbefore.cpp
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstepbefore.h
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstepnext.cpp
    ${MZ_SOURCE_DIR}/shared/tutorial/tutorialstepnext.h
    ${MZ_SOURCE_DIR}/shared/urlopener.cpp
    ${MZ_SOURCE_DIR}/shared/urlopener.h
    ${MZ_SOURCE_DIR}/shared/utils.cpp
    ${MZ_SOURCE_DIR}/shared/utils.h
    ${MZ_SOURCE_DIR}/shared/versionutils.cpp
    ${MZ_SOURCE_DIR}/shared/versionutils.h
)

# VPN Client UI resources
target_sources(qml_tests PRIVATE
    ${MZ_SOURCE_DIR}/apps/vpn/ui/ui.qrc
)

# QML test source files
target_sources(qml_tests PRIVATE
    helper.cpp
    helper.h
    main.cpp
    mocconstants.cpp
    moccontroller.cpp
    mocmozillavpn.cpp
)

if(WIN32)
    if(${Qt6_VERSION} VERSION_GREATER_EQUAL 6.3.0)
        message(WARNING "Remove the Qt6 windows hack!")
    else()
        target_sources(qml_tests PRIVATE qt6winhack.qrc)
    endif()
endif()

## Add the tests to be run.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND QML_TEST_ARGS -platform offscreen)
endif()
file(GLOB QML_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "tst_*.qml")
foreach(QML_SOURCE ${QML_TEST_SOURCES})
    add_test(NAME ${QML_SOURCE} COMMAND qml_tests ${QML_TEST_ARGS} -input ${CMAKE_CURRENT_SOURCE_DIR}/${QML_SOURCE})
endforeach()

qt6_add_qml_module(qml_tests
  URI Mozilla.Shared.qmlcomponents
  VERSION 1.0
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(qml_tests PRIVATE Qt6::DBus)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(qml_tests PRIVATE PkgConfig::libsecret)
endif()
