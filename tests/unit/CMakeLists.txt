# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/qtglean)
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/shared)
include_directories(${MZ_SOURCE_DIR}/shared/addons)
include_directories(${MZ_SOURCE_DIR}/shared/composer)
include_directories(${MZ_SOURCE_DIR}/shared/glean)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)
include_directories(${CMAKE_SOURCE_DIR}/tests/unit)

qt_add_executable(unit_tests EXCLUDE_FROM_ALL)
set_target_properties(unit_tests PROPERTIES FOLDER "Tests")
add_dependencies(build_tests unit_tests)
target_link_libraries(unit_tests PRIVATE
    Qt6::Core
    Qt6::Xml
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

target_compile_definitions(unit_tests PRIVATE UNIT_TEST)

target_link_libraries(unit_tests PRIVATE qtglean lottie nebula translations)

# VPN Client source files
target_sources(unit_tests PRIVATE
    ${MZ_SOURCE_DIR}/captiveportal/captiveportal.cpp
    ${MZ_SOURCE_DIR}/captiveportal/captiveportal.h
    ${MZ_SOURCE_DIR}/connectionhealth.cpp
    ${MZ_SOURCE_DIR}/connectionhealth.h
    ${MZ_SOURCE_DIR}/command.cpp
    ${MZ_SOURCE_DIR}/command.h
    ${MZ_SOURCE_DIR}/commandlineparser.cpp
    ${MZ_SOURCE_DIR}/commandlineparser.h
    ${MZ_SOURCE_DIR}/constants.cpp
    ${MZ_SOURCE_DIR}/constants.h
    ${MZ_SOURCE_DIR}/controller.h
    ${MZ_SOURCE_DIR}/dnshelper.cpp
    ${MZ_SOURCE_DIR}/dnshelper.h
    ${MZ_SOURCE_DIR}/dnspingsender.cpp
    ${MZ_SOURCE_DIR}/dnspingsender.h
    ${MZ_SOURCE_DIR}/ipaddresslookup.cpp
    ${MZ_SOURCE_DIR}/ipaddresslookup.h
    ${MZ_SOURCE_DIR}/models/device.cpp
    ${MZ_SOURCE_DIR}/models/device.h
    ${MZ_SOURCE_DIR}/models/devicemodel.cpp
    ${MZ_SOURCE_DIR}/models/devicemodel.h
    ${MZ_SOURCE_DIR}/models/keys.cpp
    ${MZ_SOURCE_DIR}/models/keys.h
    ${MZ_SOURCE_DIR}/models/location.cpp
    ${MZ_SOURCE_DIR}/models/location.h
    ${MZ_SOURCE_DIR}/models/recentconnections.cpp
    ${MZ_SOURCE_DIR}/models/recentconnections.h
    ${MZ_SOURCE_DIR}/models/recommendedlocationmodel.cpp
    ${MZ_SOURCE_DIR}/models/recommendedlocationmodel.h
    ${MZ_SOURCE_DIR}/models/server.cpp
    ${MZ_SOURCE_DIR}/models/server.h
    ${MZ_SOURCE_DIR}/models/servercity.cpp
    ${MZ_SOURCE_DIR}/models/servercity.h
    ${MZ_SOURCE_DIR}/models/servercountry.cpp
    ${MZ_SOURCE_DIR}/models/servercountry.h
    ${MZ_SOURCE_DIR}/models/servercountrymodel.cpp
    ${MZ_SOURCE_DIR}/models/servercountrymodel.h
    ${MZ_SOURCE_DIR}/models/serverdata.cpp
    ${MZ_SOURCE_DIR}/models/serverdata.h
    ${MZ_SOURCE_DIR}/models/subscriptiondata.cpp
    ${MZ_SOURCE_DIR}/models/subscriptiondata.h
    ${MZ_SOURCE_DIR}/models/supportcategorymodel.cpp
    ${MZ_SOURCE_DIR}/models/supportcategorymodel.h
    ${MZ_SOURCE_DIR}/models/user.cpp
    ${MZ_SOURCE_DIR}/models/user.h
    ${MZ_SOURCE_DIR}/mozillavpn.h
    ${MZ_SOURCE_DIR}/networkwatcher.cpp
    ${MZ_SOURCE_DIR}/networkwatcher.h
    ${MZ_SOURCE_DIR}/networkwatcherimpl.h
    ${MZ_SOURCE_DIR}/notificationhandler.cpp
    ${MZ_SOURCE_DIR}/notificationhandler.h
    ${MZ_SOURCE_DIR}/pinghelper.cpp
    ${MZ_SOURCE_DIR}/pinghelper.h
    ${MZ_SOURCE_DIR}/pingsender.h
    ${MZ_SOURCE_DIR}/pingsenderfactory.cpp
    ${MZ_SOURCE_DIR}/pingsenderfactory.h
    ${MZ_SOURCE_DIR}/platforms/dummy/dummynetworkwatcher.cpp
    ${MZ_SOURCE_DIR}/platforms/dummy/dummynetworkwatcher.h
    ${MZ_SOURCE_DIR}/platforms/dummy/dummypingsender.cpp
    ${MZ_SOURCE_DIR}/platforms/dummy/dummypingsender.h
    ${MZ_SOURCE_DIR}/releasemonitor.cpp
    ${MZ_SOURCE_DIR}/releasemonitor.h
    ${MZ_SOURCE_DIR}/serveri18n.cpp
    ${MZ_SOURCE_DIR}/serveri18n.h
    ${MZ_SOURCE_DIR}/serverlatency.cpp
    ${MZ_SOURCE_DIR}/serverlatency.h
    ${MZ_SOURCE_DIR}/statusicon.cpp
    ${MZ_SOURCE_DIR}/statusicon.h
    ${MZ_SOURCE_DIR}/systemtraynotificationhandler.h
    ${MZ_SOURCE_DIR}/tasks/account/taskaccount.cpp
    ${MZ_SOURCE_DIR}/tasks/account/taskaccount.h
    ${MZ_SOURCE_DIR}/tasks/adddevice/taskadddevice.cpp
    ${MZ_SOURCE_DIR}/tasks/adddevice/taskadddevice.h
    ${MZ_SOURCE_DIR}/tasks/controlleraction/taskcontrolleraction.cpp
    ${MZ_SOURCE_DIR}/tasks/controlleraction/taskcontrolleraction.h
    ${MZ_SOURCE_DIR}/tasks/ipfinder/taskipfinder.cpp
    ${MZ_SOURCE_DIR}/tasks/ipfinder/taskipfinder.h
    ${MZ_SOURCE_DIR}/tasks/release/taskrelease.cpp
    ${MZ_SOURCE_DIR}/tasks/release/taskrelease.h
    ${MZ_SOURCE_DIR}/tasks/servers/taskservers.cpp
    ${MZ_SOURCE_DIR}/tasks/servers/taskservers.h
    ${MZ_SOURCE_DIR}/update/updater.cpp
    ${MZ_SOURCE_DIR}/update/updater.h
    ${MZ_SOURCE_DIR}/update/versionapi.cpp
    ${MZ_SOURCE_DIR}/update/versionapi.h
    ${MZ_SOURCE_DIR}/update/webupdater.cpp
    ${MZ_SOURCE_DIR}/update/webupdater.h
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
    ${MZ_SOURCE_DIR}/shared/addons/addonreplacer.cpp
    ${MZ_SOURCE_DIR}/shared/addons/addonreplacer.h
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
    ${MZ_SOURCE_DIR}/shared/frontend/navigator.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigator.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarbutton.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarbutton.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarmodel.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigationbarmodel.h
    ${MZ_SOURCE_DIR}/shared/frontend/navigatorreloader.cpp
    ${MZ_SOURCE_DIR}/shared/frontend/navigatorreloader.h
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
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorhandler.cpp
    ${MZ_SOURCE_DIR}/shared/inspector/inspectoritempicker.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectoritempicker.cpp
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorhotreloader.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorhotreloader.cpp
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorwebsocketconnection.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorwebsocketconnection.cpp
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorwebsocketserver.h
    ${MZ_SOURCE_DIR}/shared/inspector/inspectorwebsocketserver.cpp
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
    ${MZ_SOURCE_DIR}/shared/networkmanager.cpp
    ${MZ_SOURCE_DIR}/shared/networkmanager.h
    ${MZ_SOURCE_DIR}/shared/networkrequest.cpp
    ${MZ_SOURCE_DIR}/shared/networkrequest.h
    ${MZ_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/qmlpath.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.h
    ${MZ_SOURCE_DIR}/shared/qmlpath.h
    ${MZ_SOURCE_DIR}/shared/resourceloader.cpp
    ${MZ_SOURCE_DIR}/shared/resourceloader.h
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
    ${MZ_SOURCE_DIR}/shared/websocket/exponentialbackoffstrategy.cpp
    ${MZ_SOURCE_DIR}/shared/websocket/exponentialbackoffstrategy.h
    ${MZ_SOURCE_DIR}/shared/websocket/pushmessage.cpp
    ${MZ_SOURCE_DIR}/shared/websocket/pushmessage.h
    ${MZ_SOURCE_DIR}/shared/websocket/websockethandler.cpp
    ${MZ_SOURCE_DIR}/shared/websocket/websockethandler.h
)

# VPN Client UI resources
target_sources(unit_tests PRIVATE
    ${MZ_SOURCE_DIR}/resources/public_keys/public_keys.qrc
)

# Unit test source files
target_sources(unit_tests PRIVATE
    main.cpp
    moccontroller.cpp
    mocmozillavpn.cpp
    mocsystemtraynotificationhandler.cpp
    helper.h
    testaddon.cpp
    testaddon.h
    testconnectionhealth.cpp
    testconnectionhealth.h
    testcommandlineparser.cpp
    testcommandlineparser.h
    testdnshelper.cpp
    testdnshelper.h
    testipaddresslookup.cpp
    testipaddresslookup.h
    testipfinder.cpp
    testipfinder.h
    testmodels.cpp
    testmodels.h
    testreleasemonitor.cpp
    testreleasemonitor.h
    testserveri18n.cpp
    testserveri18n.h
    testserverlatency.cpp
    testserverlatency.h
    teststatusicon.cpp
    teststatusicon.h
)

# Generate the version header
configure_file(${MZ_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Unit test mock resources
target_sources(unit_tests PRIVATE
    addons/addons.qrc
    servers/servers.qrc
)

## Add the tests to be run, one for each test class.
get_target_property(UTEST_SOURCES unit_tests SOURCES)
list(FILTER UTEST_SOURCES INCLUDE REGEX "test.*.h$")
foreach(filename ${UTEST_SOURCES})
    execute_process(
        OUTPUT_VARIABLE UTEST_CLASS_LIST
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/tests/list_test_classes.py -p TestHelper ${filename}
    )

    foreach(UTEST_CLASS ${UTEST_CLASS_LIST})
        add_test(NAME ${UTEST_CLASS} COMMAND unit_tests ${UTEST_CLASS})
        set_property(TEST ${UTEST_CLASS} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
    endforeach()
endforeach()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(unit_tests PRIVATE Qt6::DBus)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(unit_tests PRIVATE PkgConfig::libsecret)
endif()
