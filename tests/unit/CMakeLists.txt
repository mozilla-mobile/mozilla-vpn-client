# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DUNIT_TEST)
add_definitions(-DMVPN_ADJUST)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Test WebSockets)
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)

get_filename_component(MVPN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${MVPN_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR}/hacl-star)
include_directories(${MVPN_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${MVPN_SOURCE_DIR}/hacl-star/kremlin/minimal)

qt_add_executable(unit_tests EXCLUDE_FROM_ALL)
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

target_link_libraries(unit_tests PRIVATE glean lottie nebula translations)

# VPN Client source files
target_sources(unit_tests PRIVATE
    ${MVPN_SOURCE_DIR}/adjust/adjustfiltering.cpp
    ${MVPN_SOURCE_DIR}/adjust/adjustfiltering.h
    ${MVPN_SOURCE_DIR}/adjust/adjustproxypackagehandler.cpp
    ${MVPN_SOURCE_DIR}/adjust/adjustproxypackagehandler.h
    ${MVPN_SOURCE_DIR}/captiveportal/captiveportal.cpp
    ${MVPN_SOURCE_DIR}/captiveportal/captiveportal.h
    ${MVPN_SOURCE_DIR}/collator.cpp
    ${MVPN_SOURCE_DIR}/collator.h
    ${MVPN_SOURCE_DIR}/command.cpp
    ${MVPN_SOURCE_DIR}/command.h
    ${MVPN_SOURCE_DIR}/commandlineparser.cpp
    ${MVPN_SOURCE_DIR}/commandlineparser.h
    ${MVPN_SOURCE_DIR}/constants.cpp
    ${MVPN_SOURCE_DIR}/constants.h
    ${MVPN_SOURCE_DIR}/controller.h
    ${MVPN_SOURCE_DIR}/curve25519.cpp
    ${MVPN_SOURCE_DIR}/curve25519.h
    ${MVPN_SOURCE_DIR}/dnspingsender.cpp
    ${MVPN_SOURCE_DIR}/dnspingsender.h
    ${MVPN_SOURCE_DIR}/errorhandler.cpp
    ${MVPN_SOURCE_DIR}/errorhandler.h
    ${MVPN_SOURCE_DIR}/hacl-star/Hacl_Chacha20.c
    ${MVPN_SOURCE_DIR}/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MVPN_SOURCE_DIR}/hacl-star/Hacl_Curve25519_51.c
    ${MVPN_SOURCE_DIR}/hacl-star/Hacl_Poly1305_32.c
    ${MVPN_SOURCE_DIR}/ipaddress.cpp
    ${MVPN_SOURCE_DIR}/ipaddress.h
    ${MVPN_SOURCE_DIR}/ipaddresslookup.cpp
    ${MVPN_SOURCE_DIR}/ipaddresslookup.h
    ${MVPN_SOURCE_DIR}/itempicker.cpp
    ${MVPN_SOURCE_DIR}/itempicker.h
    ${MVPN_SOURCE_DIR}/inspector/inspectorhandler.h
    ${MVPN_SOURCE_DIR}/inspector/inspectorutils.cpp
    ${MVPN_SOURCE_DIR}/inspector/inspectorutils.h
    ${MVPN_SOURCE_DIR}/leakdetector.cpp
    ${MVPN_SOURCE_DIR}/leakdetector.h
    ${MVPN_SOURCE_DIR}/localizer.cpp
    ${MVPN_SOURCE_DIR}/localizer.h
    ${MVPN_SOURCE_DIR}/logger.cpp
    ${MVPN_SOURCE_DIR}/logger.h
    ${MVPN_SOURCE_DIR}/loghandler.cpp
    ${MVPN_SOURCE_DIR}/loghandler.h
    ${MVPN_SOURCE_DIR}/models/device.cpp
    ${MVPN_SOURCE_DIR}/models/device.h
    ${MVPN_SOURCE_DIR}/models/devicemodel.cpp
    ${MVPN_SOURCE_DIR}/models/devicemodel.h
    ${MVPN_SOURCE_DIR}/models/feature.cpp
    ${MVPN_SOURCE_DIR}/models/feature.h
    ${MVPN_SOURCE_DIR}/models/featuremodel.cpp
    ${MVPN_SOURCE_DIR}/models/featuremodel.h
    ${MVPN_SOURCE_DIR}/models/feedbackcategorymodel.cpp
    ${MVPN_SOURCE_DIR}/models/feedbackcategorymodel.h
    ${MVPN_SOURCE_DIR}/models/guide.cpp
    ${MVPN_SOURCE_DIR}/models/guide.h
    ${MVPN_SOURCE_DIR}/models/guideblock.cpp
    ${MVPN_SOURCE_DIR}/models/guideblock.h
    ${MVPN_SOURCE_DIR}/models/guidemodel.cpp
    ${MVPN_SOURCE_DIR}/models/guidemodel.h
    ${MVPN_SOURCE_DIR}/models/helpmodel.cpp
    ${MVPN_SOURCE_DIR}/models/helpmodel.h
    ${MVPN_SOURCE_DIR}/models/keys.cpp
    ${MVPN_SOURCE_DIR}/models/keys.h
    ${MVPN_SOURCE_DIR}/models/licensemodel.cpp
    ${MVPN_SOURCE_DIR}/models/licensemodel.h
    ${MVPN_SOURCE_DIR}/models/server.cpp
    ${MVPN_SOURCE_DIR}/models/server.h
    ${MVPN_SOURCE_DIR}/models/servercity.cpp
    ${MVPN_SOURCE_DIR}/models/servercity.h
    ${MVPN_SOURCE_DIR}/models/servercountry.cpp
    ${MVPN_SOURCE_DIR}/models/servercountry.h
    ${MVPN_SOURCE_DIR}/models/servercountrymodel.cpp
    ${MVPN_SOURCE_DIR}/models/servercountrymodel.h
    ${MVPN_SOURCE_DIR}/models/serverdata.cpp
    ${MVPN_SOURCE_DIR}/models/serverdata.h
    ${MVPN_SOURCE_DIR}/models/supportcategorymodel.cpp
    ${MVPN_SOURCE_DIR}/models/supportcategorymodel.h
    ${MVPN_SOURCE_DIR}/models/survey.cpp
    ${MVPN_SOURCE_DIR}/models/survey.h
    ${MVPN_SOURCE_DIR}/models/surveymodel.cpp
    ${MVPN_SOURCE_DIR}/models/surveymodel.h
    ${MVPN_SOURCE_DIR}/models/tutorial.cpp
    ${MVPN_SOURCE_DIR}/models/tutorial.h
    ${MVPN_SOURCE_DIR}/models/tutorialmodel.cpp
    ${MVPN_SOURCE_DIR}/models/tutorialmodel.h
    ${MVPN_SOURCE_DIR}/models/tutorialstep.cpp
    ${MVPN_SOURCE_DIR}/models/tutorialstep.h
    ${MVPN_SOURCE_DIR}/models/tutorialstepbefore.cpp
    ${MVPN_SOURCE_DIR}/models/tutorialstepbefore.h
    ${MVPN_SOURCE_DIR}/models/tutorialstepnext.cpp
    ${MVPN_SOURCE_DIR}/models/tutorialstepnext.h
    ${MVPN_SOURCE_DIR}/models/user.cpp
    ${MVPN_SOURCE_DIR}/models/user.h
    ${MVPN_SOURCE_DIR}/models/whatsnewmodel.cpp
    ${MVPN_SOURCE_DIR}/models/whatsnewmodel.h
    ${MVPN_SOURCE_DIR}/mozillavpn.h
    ${MVPN_SOURCE_DIR}/networkmanager.cpp
    ${MVPN_SOURCE_DIR}/networkmanager.h
    ${MVPN_SOURCE_DIR}/networkrequest.h
    ${MVPN_SOURCE_DIR}/networkwatcher.cpp
    ${MVPN_SOURCE_DIR}/networkwatcher.h
    ${MVPN_SOURCE_DIR}/networkwatcherimpl.h
    ${MVPN_SOURCE_DIR}/pinghelper.cpp
    ${MVPN_SOURCE_DIR}/pinghelper.h
    ${MVPN_SOURCE_DIR}/pingsender.h
    ${MVPN_SOURCE_DIR}/pingsenderfactory.cpp
    ${MVPN_SOURCE_DIR}/pingsenderfactory.h
    ${MVPN_SOURCE_DIR}/platforms/android/androiddatamigration.cpp
    ${MVPN_SOURCE_DIR}/platforms/android/androiddatamigration.h
    ${MVPN_SOURCE_DIR}/platforms/android/androidsharedprefs.cpp
    ${MVPN_SOURCE_DIR}/platforms/android/androidsharedprefs.h
    ${MVPN_SOURCE_DIR}/platforms/dummy/dummynetworkwatcher.cpp
    ${MVPN_SOURCE_DIR}/platforms/dummy/dummynetworkwatcher.h
    ${MVPN_SOURCE_DIR}/platforms/dummy/dummypingsender.cpp
    ${MVPN_SOURCE_DIR}/platforms/dummy/dummypingsender.h
    ${MVPN_SOURCE_DIR}/qmlengineholder.cpp
    ${MVPN_SOURCE_DIR}/qmlengineholder.h
    ${MVPN_SOURCE_DIR}/releasemonitor.cpp
    ${MVPN_SOURCE_DIR}/releasemonitor.h
    ${MVPN_SOURCE_DIR}/rfc/rfc1918.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc1918.h
    ${MVPN_SOURCE_DIR}/rfc/rfc4193.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc4193.h
    ${MVPN_SOURCE_DIR}/rfc/rfc4291.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc4291.h
    ${MVPN_SOURCE_DIR}/rfc/rfc5735.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc5735.h
    ${MVPN_SOURCE_DIR}/serveri18n.cpp
    ${MVPN_SOURCE_DIR}/serveri18n.h
    ${MVPN_SOURCE_DIR}/settingsholder.cpp
    ${MVPN_SOURCE_DIR}/settingsholder.h
    ${MVPN_SOURCE_DIR}/simplenetworkmanager.cpp
    ${MVPN_SOURCE_DIR}/simplenetworkmanager.h
    ${MVPN_SOURCE_DIR}/statusicon.cpp
    ${MVPN_SOURCE_DIR}/statusicon.h
    ${MVPN_SOURCE_DIR}/task.h
    ${MVPN_SOURCE_DIR}/tasks/account/taskaccount.cpp
    ${MVPN_SOURCE_DIR}/tasks/account/taskaccount.h
    ${MVPN_SOURCE_DIR}/tasks/adddevice/taskadddevice.cpp
    ${MVPN_SOURCE_DIR}/tasks/adddevice/taskadddevice.h
    ${MVPN_SOURCE_DIR}/tasks/ipfinder/taskipfinder.cpp
    ${MVPN_SOURCE_DIR}/tasks/ipfinder/taskipfinder.h
    ${MVPN_SOURCE_DIR}/tasks/function/taskfunction.cpp
    ${MVPN_SOURCE_DIR}/tasks/function/taskfunction.h
    ${MVPN_SOURCE_DIR}/tasks/release/taskrelease.cpp
    ${MVPN_SOURCE_DIR}/tasks/release/taskrelease.h
    ${MVPN_SOURCE_DIR}/tasks/servers/taskservers.cpp
    ${MVPN_SOURCE_DIR}/tasks/servers/taskservers.h
    ${MVPN_SOURCE_DIR}/taskscheduler.cpp
    ${MVPN_SOURCE_DIR}/taskscheduler.h
    ${MVPN_SOURCE_DIR}/theme.cpp
    ${MVPN_SOURCE_DIR}/theme.h
    ${MVPN_SOURCE_DIR}/timersingleshot.cpp
    ${MVPN_SOURCE_DIR}/timersingleshot.h
    ${MVPN_SOURCE_DIR}/update/updater.cpp
    ${MVPN_SOURCE_DIR}/update/updater.h
    ${MVPN_SOURCE_DIR}/update/versionapi.cpp
    ${MVPN_SOURCE_DIR}/update/versionapi.h
    ${MVPN_SOURCE_DIR}/urlopener.cpp
    ${MVPN_SOURCE_DIR}/urlopener.h
)

# VPN Client UI resources
target_sources(unit_tests PRIVATE
    ${MVPN_SOURCE_DIR}/ui/license.qrc
)

# Unit test source files
target_sources(unit_tests PRIVATE
    main.cpp
    moccontroller.cpp
    mocinspectorhandler.cpp
    mocmozillavpn.cpp
    mocnetworkrequest.cpp
    helper.h
    testadjust.cpp
    testadjust.h
    testandroidmigration.cpp
    testandroidmigration.h
    testcommandlineparser.cpp
    testcommandlineparser.h
    testfeature.cpp
    testfeature.h
    testguide.cpp
    testguide.h
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
    teststatusicon.cpp
    teststatusicon.h
    testtasks.cpp
    testtasks.h
    testthemes.cpp
    testthemes.h
    testtimersingleshot.cpp
    testtimersingleshot.h
    testtutorial.cpp
    testtutorial.h
)

# Generate the version header
configure_file(${MVPN_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(auth_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Unit test mock resources
target_sources(unit_tests PRIVATE
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
        COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/tests/list_test_classes.py -p TestHelper ${filename}
    )

    foreach(UTEST_CLASS ${UTEST_CLASS_LIST})
        add_test(NAME ${UTEST_CLASS} COMMAND unit_tests ${UTEST_CLASS})
        set_property(TEST ${UTEST_CLASS} PROPERTY ENVIRONMENT LANG="en" LANGUAGE="en")
    endforeach()
endforeach()
