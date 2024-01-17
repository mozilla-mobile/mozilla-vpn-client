# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/qtglean)
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/addons)
include_directories(${MZ_SOURCE_DIR}/composer)
include_directories(${MZ_SOURCE_DIR}/glean)
include_directories(${MZ_SOURCE_DIR}/hacl-star)
include_directories(${MZ_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${MZ_SOURCE_DIR}/hacl-star/kremlin/minimal)
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
    ${MZ_SOURCE_DIR}/tcppingsender.cpp
    ${MZ_SOURCE_DIR}/tcppingsender.h
    ${MZ_SOURCE_DIR}/update/updater.cpp
    ${MZ_SOURCE_DIR}/update/updater.h
    ${MZ_SOURCE_DIR}/update/versionapi.cpp
    ${MZ_SOURCE_DIR}/update/versionapi.h
    ${MZ_SOURCE_DIR}/update/webupdater.cpp
    ${MZ_SOURCE_DIR}/update/webupdater.h
    ${MZ_SOURCE_DIR}/addons/addon.cpp
    ${MZ_SOURCE_DIR}/addons/addon.h
    ${MZ_SOURCE_DIR}/addons/addonapi.cpp
    ${MZ_SOURCE_DIR}/addons/addonapi.h
    ${MZ_SOURCE_DIR}/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/addons/addonguide.cpp
    ${MZ_SOURCE_DIR}/addons/addonguide.h
    ${MZ_SOURCE_DIR}/addons/addoni18n.cpp
    ${MZ_SOURCE_DIR}/addons/addoni18n.h
    ${MZ_SOURCE_DIR}/addons/addonmessage.cpp
    ${MZ_SOURCE_DIR}/addons/addonmessage.h
    ${MZ_SOURCE_DIR}/addons/addonproperty.cpp
    ${MZ_SOURCE_DIR}/addons/addonproperty.h
    ${MZ_SOURCE_DIR}/addons/addonpropertylist.cpp
    ${MZ_SOURCE_DIR}/addons/addonpropertylist.h
    ${MZ_SOURCE_DIR}/addons/addonreplacer.cpp
    ${MZ_SOURCE_DIR}/addons/addonreplacer.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcher.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcher.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchergroup.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchergroup.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherjavascript.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherjavascript.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherlocales.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatcherlocales.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertime.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertime.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertimeend.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertimestart.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertranslationthreshold.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertranslationthreshold.h
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertriggertimesecs.cpp
    ${MZ_SOURCE_DIR}/addons/conditionwatchers/addonconditionwatchertriggertimesecs.h
    ${MZ_SOURCE_DIR}/addons/manager/addondirectory.cpp
    ${MZ_SOURCE_DIR}/addons/manager/addondirectory.h
    ${MZ_SOURCE_DIR}/addons/manager/addonindex.cpp
    ${MZ_SOURCE_DIR}/addons/manager/addonindex.h
    ${MZ_SOURCE_DIR}/addons/manager/addonmanager.cpp
    ${MZ_SOURCE_DIR}/addons/manager/addonmanager.h
    ${MZ_SOURCE_DIR}/addons/state/addonstate.h
    ${MZ_SOURCE_DIR}/addons/state/addonstatebase.cpp
    ${MZ_SOURCE_DIR}/addons/state/addonstatebase.h
    ${MZ_SOURCE_DIR}/addons/state/addonsessionstate.cpp
    ${MZ_SOURCE_DIR}/addons/state/addonsessionstate.h
    ${MZ_SOURCE_DIR}/app.cpp
    ${MZ_SOURCE_DIR}/app.h
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinapp.cpp
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinapp.h
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.cpp
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.h
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinappsession.cpp
    ${MZ_SOURCE_DIR}/authenticationinapp/authenticationinappsession.h
    ${MZ_SOURCE_DIR}/authenticationinapp/incrementaldecoder.cpp
    ${MZ_SOURCE_DIR}/authenticationinapp/incrementaldecoder.h
    ${MZ_SOURCE_DIR}/authenticationlistener.cpp
    ${MZ_SOURCE_DIR}/authenticationlistener.h
    ${MZ_SOURCE_DIR}/collator.cpp
    ${MZ_SOURCE_DIR}/collator.h
    ${MZ_SOURCE_DIR}/composer/composer.cpp
    ${MZ_SOURCE_DIR}/composer/composer.h
    ${MZ_SOURCE_DIR}/composer/composerblock.cpp
    ${MZ_SOURCE_DIR}/composer/composerblock.h
    ${MZ_SOURCE_DIR}/composer/composerblockbutton.cpp
    ${MZ_SOURCE_DIR}/composer/composerblockbutton.h
    ${MZ_SOURCE_DIR}/composer/composerblockorderedlist.cpp
    ${MZ_SOURCE_DIR}/composer/composerblockorderedlist.h
    ${MZ_SOURCE_DIR}/composer/composerblocktext.cpp
    ${MZ_SOURCE_DIR}/composer/composerblocktext.h
    ${MZ_SOURCE_DIR}/composer/composerblocktitle.cpp
    ${MZ_SOURCE_DIR}/composer/composerblocktitle.h
    ${MZ_SOURCE_DIR}/composer/composerblockunorderedlist.cpp
    ${MZ_SOURCE_DIR}/composer/composerblockunorderedlist.h
    ${MZ_SOURCE_DIR}/cryptosettings.cpp
    ${MZ_SOURCE_DIR}/cryptosettings.h
    ${MZ_SOURCE_DIR}/curve25519.cpp
    ${MZ_SOURCE_DIR}/curve25519.h
    ${MZ_SOURCE_DIR}/env.cpp
    ${MZ_SOURCE_DIR}/env.h
    ${MZ_SOURCE_DIR}/errorhandler.cpp
    ${MZ_SOURCE_DIR}/errorhandler.h
    ${MZ_SOURCE_DIR}/feature/feature.cpp
    ${MZ_SOURCE_DIR}/feature/feature.h
    ${MZ_SOURCE_DIR}/frontend/navigator.cpp
    ${MZ_SOURCE_DIR}/frontend/navigator.h
    ${MZ_SOURCE_DIR}/frontend/navigationbarbutton.cpp
    ${MZ_SOURCE_DIR}/frontend/navigationbarbutton.h
    ${MZ_SOURCE_DIR}/frontend/navigationbarmodel.cpp
    ${MZ_SOURCE_DIR}/frontend/navigationbarmodel.h
    ${MZ_SOURCE_DIR}/frontend/navigatorreloader.cpp
    ${MZ_SOURCE_DIR}/frontend/navigatorreloader.h
    ${MZ_SOURCE_DIR}/glean/mzglean.h
    ${MZ_SOURCE_DIR}/glean/mzglean.cpp
    ${MZ_SOURCE_DIR}/hacl-star/Hacl_Chacha20.c
    ${MZ_SOURCE_DIR}/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MZ_SOURCE_DIR}/hacl-star/Hacl_Curve25519_51.c
    ${MZ_SOURCE_DIR}/hacl-star/Hacl_Poly1305_32.c
    ${MZ_SOURCE_DIR}/hawkauth.cpp
    ${MZ_SOURCE_DIR}/hawkauth.h
    ${MZ_SOURCE_DIR}/hkdf.cpp
    ${MZ_SOURCE_DIR}/hkdf.h
    ${MZ_SOURCE_DIR}/inspector/inspectorhandler.h
    ${MZ_SOURCE_DIR}/inspector/inspectorhandler.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectoritempicker.h
    ${MZ_SOURCE_DIR}/inspector/inspectoritempicker.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectorhotreloader.h
    ${MZ_SOURCE_DIR}/inspector/inspectorhotreloader.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectorwebsocketconnection.h
    ${MZ_SOURCE_DIR}/inspector/inspectorwebsocketconnection.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectorwebsocketserver.h
    ${MZ_SOURCE_DIR}/inspector/inspectorwebsocketserver.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectorutils.cpp
    ${MZ_SOURCE_DIR}/inspector/inspectorutils.h
    ${MZ_SOURCE_DIR}/ipaddress.cpp
    ${MZ_SOURCE_DIR}/ipaddress.h
    ${MZ_SOURCE_DIR}/itempicker.cpp
    ${MZ_SOURCE_DIR}/itempicker.h
    ${MZ_SOURCE_DIR}/languagei18n.cpp
    ${MZ_SOURCE_DIR}/languagei18n.h
    ${MZ_SOURCE_DIR}/leakdetector.cpp
    ${MZ_SOURCE_DIR}/leakdetector.h
    ${MZ_SOURCE_DIR}/localizer.cpp
    ${MZ_SOURCE_DIR}/localizer.h
    ${MZ_SOURCE_DIR}/logger.cpp
    ${MZ_SOURCE_DIR}/logger.h
    ${MZ_SOURCE_DIR}/loghandler.cpp
    ${MZ_SOURCE_DIR}/loghandler.h
    ${MZ_SOURCE_DIR}/feature/featuremodel.cpp
    ${MZ_SOURCE_DIR}/feature/featuremodel.h
    ${MZ_SOURCE_DIR}/models/licensemodel.cpp
    ${MZ_SOURCE_DIR}/models/licensemodel.h
    ${MZ_SOURCE_DIR}/networkmanager.cpp
    ${MZ_SOURCE_DIR}/networkmanager.h
    ${MZ_SOURCE_DIR}/networkrequest.cpp
    ${MZ_SOURCE_DIR}/networkrequest.h
    ${MZ_SOURCE_DIR}/platforms/wasm/wasmcryptosettings.cpp
    ${MZ_SOURCE_DIR}/qmlpath.cpp
    ${MZ_SOURCE_DIR}/qmlengineholder.cpp
    ${MZ_SOURCE_DIR}/qmlengineholder.h
    ${MZ_SOURCE_DIR}/qmlpath.h
    ${MZ_SOURCE_DIR}/resourceloader.cpp
    ${MZ_SOURCE_DIR}/resourceloader.h
    ${MZ_SOURCE_DIR}/rfc/rfc1918.cpp
    ${MZ_SOURCE_DIR}/rfc/rfc1918.h
    ${MZ_SOURCE_DIR}/rfc/rfc4193.cpp
    ${MZ_SOURCE_DIR}/rfc/rfc4193.h
    ${MZ_SOURCE_DIR}/rfc/rfc4291.cpp
    ${MZ_SOURCE_DIR}/rfc/rfc4291.h
    ${MZ_SOURCE_DIR}/rfc/rfc5735.cpp
    ${MZ_SOURCE_DIR}/rfc/rfc5735.h
    ${MZ_SOURCE_DIR}/settings/settinggroup.cpp
    ${MZ_SOURCE_DIR}/settings/settinggroup.h
    ${MZ_SOURCE_DIR}/settings/settingsmanager.cpp
    ${MZ_SOURCE_DIR}/settings/settingsmanager.h
    ${CMAKE_SOURCE_DIR}/src/settings/settingsconnector.h
    ${CMAKE_SOURCE_DIR}/src/settings/settingsconnector.cpp
    ${MZ_SOURCE_DIR}/settings/setting.cpp
    ${MZ_SOURCE_DIR}/settings/setting.h
    ${MZ_SOURCE_DIR}/settingsholder.cpp
    ${MZ_SOURCE_DIR}/settingsholder.h
    ${MZ_SOURCE_DIR}/simplenetworkmanager.cpp
    ${MZ_SOURCE_DIR}/simplenetworkmanager.h
    ${MZ_SOURCE_DIR}/signature.cpp
    ${MZ_SOURCE_DIR}/signature.h
    ${MZ_SOURCE_DIR}/task.h
    ${MZ_SOURCE_DIR}/taskscheduler.cpp
    ${MZ_SOURCE_DIR}/taskscheduler.h
    ${MZ_SOURCE_DIR}/tasks/addon/taskaddon.cpp
    ${MZ_SOURCE_DIR}/tasks/addon/taskaddon.h
    ${MZ_SOURCE_DIR}/tasks/addonindex/taskaddonindex.cpp
    ${MZ_SOURCE_DIR}/tasks/addonindex/taskaddonindex.h
    ${MZ_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.cpp
    ${MZ_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.h
    ${MZ_SOURCE_DIR}/tasks/function/taskfunction.cpp
    ${MZ_SOURCE_DIR}/tasks/function/taskfunction.h
    ${MZ_SOURCE_DIR}/tasks/group/taskgroup.cpp
    ${MZ_SOURCE_DIR}/tasks/group/taskgroup.h
    ${MZ_SOURCE_DIR}/temporarydir.cpp
    ${MZ_SOURCE_DIR}/temporarydir.h
    ${MZ_SOURCE_DIR}/theme.cpp
    ${MZ_SOURCE_DIR}/theme.h
    ${MZ_SOURCE_DIR}/urlopener.cpp
    ${MZ_SOURCE_DIR}/urlopener.h
    ${MZ_SOURCE_DIR}/utils.cpp
    ${MZ_SOURCE_DIR}/utils.h
    ${MZ_SOURCE_DIR}/versionutils.cpp
    ${MZ_SOURCE_DIR}/versionutils.h
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
