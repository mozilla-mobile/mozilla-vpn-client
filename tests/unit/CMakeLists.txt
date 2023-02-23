# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/qtglean)
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/shared)
include_directories(${MZ_SOURCE_DIR}/shared/glean)
include_directories(${MZ_SOURCE_DIR}/apps/vpn)
include_directories(${MZ_SOURCE_DIR}/apps/vpn/composer)
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
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

target_compile_definitions(unit_tests PRIVATE UNIT_TEST)
target_compile_definitions(unit_tests PRIVATE MVPN_ADJUST)

target_link_libraries(unit_tests PRIVATE glean qtglean lottie nebula translations)

# VPN Client source files
target_sources(unit_tests PRIVATE
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addon.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addon.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonapi.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonapi.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonguide.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addoni18n.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addoni18n.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonmessage.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonmessage.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonproperty.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonproperty.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonpropertylist.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addonpropertylist.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addontutorial.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/addontutorial.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcher.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcher.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchergroup.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertime.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addondirectory.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addondirectory.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addonindex.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addonindex.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addonmanager.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/manager/addonmanager.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/state/addonstate.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/state/addonstatebase.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/state/addonstatebase.h
    ${MZ_SOURCE_DIR}/apps/vpn/addons/state/addonsessionstate.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/addons/state/addonsessionstate.h
    ${MZ_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.h
    ${MZ_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/appconstants.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/appconstants.h
    ${MZ_SOURCE_DIR}/apps/vpn/captiveportal/captiveportal.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/captiveportal/captiveportal.h
    ${MZ_SOURCE_DIR}/apps/vpn/command.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/command.h
    ${MZ_SOURCE_DIR}/apps/vpn/commandlineparser.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/commandlineparser.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composer.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composer.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblock.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblock.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockbutton.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockbutton.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockorderedlist.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockorderedlist.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblocktext.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblocktext.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblocktitle.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblocktitle.h
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockunorderedlist.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/composer/composerblockunorderedlist.h
    ${MZ_SOURCE_DIR}/apps/vpn/controller.h
    ${MZ_SOURCE_DIR}/apps/vpn/dnshelper.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/dnshelper.h
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/errorhandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/errorhandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/externalophandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/externalophandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/frontend/navigator.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/frontend/navigator.h
    ${MZ_SOURCE_DIR}/apps/vpn/frontend/navigatorreloader.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/frontend/navigatorreloader.h
    ${MZ_SOURCE_DIR}/apps/vpn/inspector/inspectorhandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/inspector/inspectorutils.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/inspector/inspectorutils.h
    ${MZ_SOURCE_DIR}/apps/vpn/ipaddresslookup.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/ipaddresslookup.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/device.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/device.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/devicemodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/devicemodel.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/featuremodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/featuremodel.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/feedbackcategorymodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/feedbackcategorymodel.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/keys.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/keys.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/location.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/location.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/recentconnections.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/recentconnections.h
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
    ${MZ_SOURCE_DIR}/apps/vpn/models/supportcategorymodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/supportcategorymodel.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/user.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/user.h
    ${MZ_SOURCE_DIR}/apps/vpn/mozillavpn.h
    ${MZ_SOURCE_DIR}/apps/vpn/networkwatcher.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/networkwatcher.h
    ${MZ_SOURCE_DIR}/apps/vpn/networkwatcherimpl.h
    ${MZ_SOURCE_DIR}/apps/vpn/notificationhandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/notificationhandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/pinghelper.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/pinghelper.h
    ${MZ_SOURCE_DIR}/apps/vpn/pingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/pingsenderfactory.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/pingsenderfactory.h
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummynetworkwatcher.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummynetworkwatcher.h
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/releasemonitor.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/releasemonitor.h
    ${MZ_SOURCE_DIR}/apps/vpn/serveri18n.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/serveri18n.h
    ${MZ_SOURCE_DIR}/apps/vpn/serverlatency.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/serverlatency.h
    ${MZ_SOURCE_DIR}/apps/vpn/sentry/sentryadapter.h
    ${MZ_SOURCE_DIR}/apps/vpn/statusicon.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/statusicon.h
    ${MZ_SOURCE_DIR}/apps/vpn/systemtraynotificationhandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/account/taskaccount.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/account/taskaccount.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/adddevice/taskadddevice.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/adddevice/taskadddevice.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/addon/taskaddon.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/addon/taskaddon.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/addonindex/taskaddonindex.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/addonindex/taskaddonindex.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/controlleraction/taskcontrolleraction.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/controlleraction/taskcontrolleraction.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/ipfinder/taskipfinder.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/ipfinder/taskipfinder.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/release/taskrelease.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/release/taskrelease.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/servers/taskservers.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/servers/taskservers.h
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/sentry/tasksentry.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tasks/sentry/tasksentry.h
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorial.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorial.h
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstep.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstep.h
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepbefore.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepbefore.h
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepnext.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/tutorial/tutorialstepnext.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/updater.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/updater.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/versionapi.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/versionapi.h
    ${MZ_SOURCE_DIR}/apps/vpn/update/webupdater.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/update/webupdater.h
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/exponentialbackoffstrategy.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/exponentialbackoffstrategy.h
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/pushmessage.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/pushmessage.h
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/websockethandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/websocket/websockethandler.h
    ${MZ_SOURCE_DIR}/shared/collator.cpp
    ${MZ_SOURCE_DIR}/shared/collator.h
    ${MZ_SOURCE_DIR}/shared/constants.cpp
    ${MZ_SOURCE_DIR}/shared/constants.h
    ${MZ_SOURCE_DIR}/shared/cryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/cryptosettings.h
    ${MZ_SOURCE_DIR}/shared/curve25519.cpp
    ${MZ_SOURCE_DIR}/shared/curve25519.h
    ${MZ_SOURCE_DIR}/shared/env.h
    ${MZ_SOURCE_DIR}/shared/feature.cpp
    ${MZ_SOURCE_DIR}/shared/feature.h
    ${MZ_SOURCE_DIR}/shared/glean/gleandeprecated.cpp
    ${MZ_SOURCE_DIR}/shared/glean/gleandeprecated.h
    ${MZ_SOURCE_DIR}/shared/glean/mzglean.h
    ${MZ_SOURCE_DIR}/shared/glean/mzglean.cpp
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${MZ_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
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
    ${MZ_SOURCE_DIR}/shared/networkmanager.cpp
    ${MZ_SOURCE_DIR}/shared/networkmanager.h
    ${MZ_SOURCE_DIR}/shared/networkrequest.cpp
    ${MZ_SOURCE_DIR}/shared/networkrequest.h
    ${MZ_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/qmlpath.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.h
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
    ${MZ_SOURCE_DIR}/shared/tasks/function/taskfunction.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/function/taskfunction.h
    ${MZ_SOURCE_DIR}/shared/tasks/group/taskgroup.cpp
    ${MZ_SOURCE_DIR}/shared/tasks/group/taskgroup.h
    ${MZ_SOURCE_DIR}/shared/temporarydir.cpp
    ${MZ_SOURCE_DIR}/shared/temporarydir.h
    ${MZ_SOURCE_DIR}/shared/theme.cpp
    ${MZ_SOURCE_DIR}/shared/theme.h
    ${MZ_SOURCE_DIR}/shared/urlopener.cpp
    ${MZ_SOURCE_DIR}/shared/urlopener.h
    ${MZ_SOURCE_DIR}/shared/versionutils.cpp
    ${MZ_SOURCE_DIR}/shared/versionutils.h
)

# VPN Client UI resources
target_sources(unit_tests PRIVATE
    ${MZ_SOURCE_DIR}/apps/vpn/resources/public_keys/public_keys.qrc
)

# Unit test source files
target_sources(unit_tests PRIVATE
    main.cpp
    moccontroller.cpp
    mocinspectorhandler.cpp
    mocmozillavpn.cpp
    mocsystemtraynotificationhandler.cpp
    mocsentryadapter.cpp
    helper.h
    testaddon.cpp
    testaddon.h
    testaddonapi.cpp
    testaddonapi.h
    testaddonindex.cpp
    testaddonindex.h
    testaddonstatebase.cpp
    testaddonstatebase.h
    testaddonsessionstate.cpp
    testaddonsessionstate.h
    testadjust.cpp
    testadjust.h
    testcommandlineparser.cpp
    testcommandlineparser.h
    testcomposer.cpp
    testcomposer.h
    testdnshelper.cpp
    testdnshelper.h
    testfeaturemodel.cpp
    testfeaturemodel.h
    testipaddresslookup.cpp
    testipaddresslookup.h
    testipfinder.cpp
    testipfinder.h
    testmodels.cpp
    testmodels.h
    testmozillavpnh.cpp
    testmozillavpnh.h
    testreleasemonitor.cpp
    testreleasemonitor.h
    testserveri18n.cpp
    testserveri18n.h
    testserverlatency.cpp
    testserverlatency.h
    teststatusicon.cpp
    teststatusicon.h
    testtasksentry.cpp
    testtasksentry.h
    websocket/testexponentialbackoffstrategy.cpp
    websocket/testexponentialbackoffstrategy.h
    websocket/testpushmessage.cpp
    websocket/testpushmessage.h
    websocket/testwebsockethandler.cpp
    websocket/testwebsockethandler.h
)

# Generate the version header
configure_file(${MZ_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(auth_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Unit test mock resources
target_sources(unit_tests PRIVATE
    addons/addons.qrc
    guides/guides.qrc
    servers/servers.qrc
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
