# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MVPN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/vpnglean)
include_directories(${MVPN_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR}/shared)
include_directories(${MVPN_SOURCE_DIR}/apps/vpn)
include_directories(${MVPN_SOURCE_DIR}/apps/vpn/composer)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)
include_directories(${CMAKE_SOURCE_DIR}/tests/unit)

qt_add_executable(unit_tests EXCLUDE_FROM_ALL)
set_target_properties(unit_tests PROPERTIES FOLDER "Tests")
add_dependencies(build_tests unit_tests)
target_link_libraries(unit_tests PRIVATE
    Qt6::Core
    Qt6::Xml
    Qt6::Network
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

target_compile_definitions(unit_tests PRIVATE UNIT_TEST)
target_compile_definitions(unit_tests PRIVATE MVPN_ADJUST)

target_link_libraries(unit_tests PRIVATE glean lottie nebula translations)
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    target_link_libraries(unit_tests PRIVATE vpnglean)
endif()

# VPN Client source files
target_sources(unit_tests PRIVATE
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addon.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addon.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonapi.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonapi.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonguide.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonguide.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonguide.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addoni18n.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addoni18n.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonmessage.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonmessage.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonproperty.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonproperty.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonpropertylist.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addonpropertylist.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addontutorial.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/addontutorial.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcher.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcher.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchergroup.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertime.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addondirectory.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addondirectory.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addonindex.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addonindex.h
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addonmanager.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/addons/manager/addonmanager.h
    ${MVPN_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.h
    ${MVPN_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/captiveportal/captiveportal.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/captiveportal/captiveportal.h
    ${MVPN_SOURCE_DIR}/apps/vpn/collator.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/collator.h
    ${MVPN_SOURCE_DIR}/apps/vpn/command.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/command.h
    ${MVPN_SOURCE_DIR}/apps/vpn/commandlineparser.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/commandlineparser.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composer.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composer.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblock.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblock.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockbutton.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockbutton.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockorderedlist.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockorderedlist.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblocktext.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblocktext.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblocktitle.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblocktitle.h
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockunorderedlist.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/composer/composerblockunorderedlist.h
    ${MVPN_SOURCE_DIR}/apps/vpn/constants.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/constants.h
    ${MVPN_SOURCE_DIR}/apps/vpn/controller.h
    ${MVPN_SOURCE_DIR}/apps/vpn/cryptosettings.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/cryptosettings.h
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/env.h
    ${MVPN_SOURCE_DIR}/apps/vpn/errorhandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/errorhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/externalophandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/externalophandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/frontend/navigator.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/frontend/navigator.h
    ${MVPN_SOURCE_DIR}/apps/vpn/frontend/navigatorreloader.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/frontend/navigatorreloader.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/glean.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/glean.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/generated/metrics.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/generated/pings.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/private/event.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/private/event.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/private/ping.h
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/private/ping.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/glean/metrictypes.h
    ${MVPN_SOURCE_DIR}/apps/vpn/inspector/inspectorhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/inspector/inspectorutils.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/inspector/inspectorutils.h
    ${MVPN_SOURCE_DIR}/apps/vpn/ipaddresslookup.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/ipaddresslookup.h
    ${MVPN_SOURCE_DIR}/apps/vpn/itempicker.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/itempicker.h
    ${MVPN_SOURCE_DIR}/apps/vpn/localizer.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/localizer.h
    ${MVPN_SOURCE_DIR}/apps/vpn/logger.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/logger.h
    ${MVPN_SOURCE_DIR}/apps/vpn/loghandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/loghandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/device.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/device.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/devicemodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/devicemodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feature.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feature.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/featuremodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/featuremodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feedbackcategorymodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feedbackcategorymodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/keys.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/keys.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/licensemodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/licensemodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/recentconnections.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/recentconnections.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/server.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/server.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercity.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercity.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercountry.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercountry.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercountrymodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/servercountrymodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/serverdata.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/serverdata.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/subscriptiondata.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/subscriptiondata.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/supportcategorymodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/supportcategorymodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/user.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/user.h
    ${MVPN_SOURCE_DIR}/apps/vpn/mozillavpn.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkmanager.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/networkmanager.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkrequest.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkwatcher.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/networkwatcher.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkwatcherimpl.h
    ${MVPN_SOURCE_DIR}/apps/vpn/notificationhandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/notificationhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.h
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummycryptosettings.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummynetworkwatcher.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummynetworkwatcher.h
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/qmlengineholder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/qmlengineholder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/releasemonitor.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/releasemonitor.h
    ${MVPN_SOURCE_DIR}/apps/vpn/serveri18n.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/serveri18n.h
    ${MVPN_SOURCE_DIR}/apps/vpn/settingsholder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/settingsholder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/signature.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/signature.h
    ${MVPN_SOURCE_DIR}/apps/vpn/simplenetworkmanager.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/simplenetworkmanager.h
    ${MVPN_SOURCE_DIR}/apps/vpn/statusicon.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/statusicon.h
    ${MVPN_SOURCE_DIR}/apps/vpn/systemtraynotificationhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/task.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/account/taskaccount.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/account/taskaccount.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/adddevice/taskadddevice.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/adddevice/taskadddevice.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/addon/taskaddon.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/addon/taskaddon.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/addonindex/taskaddonindex.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/addonindex/taskaddonindex.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/function/taskfunction.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/function/taskfunction.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/group/taskgroup.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/group/taskgroup.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/ipfinder/taskipfinder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/ipfinder/taskipfinder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/release/taskrelease.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/release/taskrelease.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/servers/taskservers.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/servers/taskservers.h
    ${MVPN_SOURCE_DIR}/apps/vpn/taskscheduler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/taskscheduler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/temporarydir.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/temporarydir.h
    ${MVPN_SOURCE_DIR}/apps/vpn/theme.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/theme.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorial.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorial.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstep.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstep.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepbefore.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepbefore.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepnext.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepnext.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.h
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/exponentialbackoffstrategy.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/exponentialbackoffstrategy.h
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/pushmessage.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/pushmessage.h
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/websockethandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/websocket/websockethandler.h
    ${MVPN_SOURCE_DIR}/shared/curve25519.cpp
    ${MVPN_SOURCE_DIR}/shared/curve25519.h
    ${MVPN_SOURCE_DIR}/shared/ipaddress.cpp
    ${MVPN_SOURCE_DIR}/shared/ipaddress.h
    ${MVPN_SOURCE_DIR}/shared/leakdetector.cpp
    ${MVPN_SOURCE_DIR}/shared/leakdetector.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc1918.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc1918.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4193.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4193.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4291.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4291.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc5735.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc5735.h
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
)

# VPN Client UI resources
target_sources(unit_tests PRIVATE
    ${MVPN_SOURCE_DIR}/apps/vpn/ui/license.qrc
    ${MVPN_SOURCE_DIR}/apps/vpn/resources/public_keys/public_keys.qrc
)

# Unit test source files
target_sources(unit_tests PRIVATE
    main.cpp
    moccontroller.cpp
    mocinspectorhandler.cpp
    mocmozillavpn.cpp
    mocnetworkrequest.cpp
    mocsystemtraynotificationhandler.cpp
    helper.h
    testaddon.cpp
    testaddon.h
    testaddonapi.cpp
    testaddonapi.h
    testaddonindex.cpp
    testaddonindex.h
    testadjust.cpp
    testadjust.h
    testcheckedint.h
    testcheckedint.cpp
    testcommandlineparser.cpp
    testcommandlineparser.h
    testcomposer.cpp
    testcomposer.h
    testfeature.cpp
    testfeature.h
    testipaddress.cpp
    testipaddress.h
    testipaddresslookup.cpp
    testipaddresslookup.h
    testipfinder.cpp
    testipfinder.h
    testlicense.cpp
    testlicense.h
    testlocalizer.cpp
    testlocalizer.h
    testlogger.cpp
    testlogger.h
    testmodels.cpp
    testmodels.h
    testmozillavpnh.cpp
    testmozillavpnh.h
    testnetworkmanager.cpp
    testnetworkmanager.h
    testreleasemonitor.cpp
    testreleasemonitor.h
    testserveri18n.cpp
    testserveri18n.h
    testsettings.cpp
    testsettings.h
    teststatusicon.cpp
    teststatusicon.h
    testtasks.cpp
    testtasks.h
    testtemporarydir.cpp
    testtemporarydir.h
    testthemes.cpp
    testthemes.h
    testurlopener.cpp
    testurlopener.h
    websocket/testexponentialbackoffstrategy.cpp
    websocket/testexponentialbackoffstrategy.h
    websocket/testpushmessage.cpp
    websocket/testpushmessage.h
    websocket/testwebsockethandler.cpp
    websocket/testwebsockethandler.h
)

# Generate the version header
configure_file(${MVPN_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(auth_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Unit test mock resources
target_sources(unit_tests PRIVATE
    addons/addons.qrc
    guides/guides.qrc
    servers/servers.qrc
    themes/themes.qrc
    tutorials/tutorials.qrc
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
