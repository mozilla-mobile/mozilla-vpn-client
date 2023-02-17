# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR})
include_directories(${MZ_SOURCE_DIR}/apps/vpn)
include_directories(${MZ_SOURCE_DIR}/shared)
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
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickTest
)

target_link_libraries(qml_tests PRIVATE glean qtglean lottie nebula translations)

target_compile_definitions(qml_tests PRIVATE QUICK_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
target_compile_definitions(qml_tests PRIVATE UNIT_TEST)
target_compile_definitions(qml_tests PRIVATE MZ_DUMMY)

# VPN Client source files
target_sources(qml_tests PRIVATE
    ${MZ_SOURCE_DIR}/apps/vpn/appconstants.h
    ${MZ_SOURCE_DIR}/apps/vpn/controller.h
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MZ_SOURCE_DIR}/apps/vpn/externalophandler.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/externalophandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/inspector/inspectorhandler.h
    ${MZ_SOURCE_DIR}/apps/vpn/models/featuremodel.cpp
    ${MZ_SOURCE_DIR}/apps/vpn/models/featuremodel.h
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
    ${MZ_SOURCE_DIR}/shared/collator.cpp
    ${MZ_SOURCE_DIR}/shared/collator.h
    ${MZ_SOURCE_DIR}/shared/constants.h
    ${MZ_SOURCE_DIR}/shared/cryptosettings.cpp
    ${MZ_SOURCE_DIR}/shared/cryptosettings.h
    ${MZ_SOURCE_DIR}/shared/env.h
    ${MZ_SOURCE_DIR}/shared/feature.cpp
    ${MZ_SOURCE_DIR}/shared/feature.h
    ${MZ_SOURCE_DIR}/shared/filterproxymodel.cpp
    ${MZ_SOURCE_DIR}/shared/filterproxymodel.h
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
    ${MZ_SOURCE_DIR}/shared/ipaddress.cpp
    ${MZ_SOURCE_DIR}/shared/ipaddress.h
    ${MZ_SOURCE_DIR}/shared/languagei18n.cpp
    ${MZ_SOURCE_DIR}/shared/languagei18n.h
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
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.cpp
    ${MZ_SOURCE_DIR}/shared/qmlengineholder.h
    ${MZ_SOURCE_DIR}/shared/settingsholder.cpp
    ${MZ_SOURCE_DIR}/shared/settingsholder.h
    ${MZ_SOURCE_DIR}/shared/task.h
    ${MZ_SOURCE_DIR}/shared/taskscheduler.cpp
    ${MZ_SOURCE_DIR}/shared/taskscheduler.h
    ${MZ_SOURCE_DIR}/shared/theme.cpp
    ${MZ_SOURCE_DIR}/shared/theme.h
    ${MZ_SOURCE_DIR}/shared/urlopener.cpp
    ${MZ_SOURCE_DIR}/shared/urlopener.h
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
    ../unit/mocinspectorhandler.cpp
)

## Add the tests to be run.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND QML_TEST_ARGS -platform offscreen)
endif()
file(GLOB QML_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "tst_*.qml")
foreach(QML_SOURCE ${QML_TEST_SOURCES})
    add_test(NAME ${QML_SOURCE} COMMAND qml_tests ${QML_TEST_ARGS} -input ${CMAKE_CURRENT_SOURCE_DIR}/${QML_SOURCE})
endforeach()

qt6_add_qml_module(qml_tests
  URI Mozilla.VPN.qmlcomponents
  VERSION 1.0
)
