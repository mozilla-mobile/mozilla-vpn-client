# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MVPN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR}/apps/vpn)
include_directories(${MVPN_SOURCE_DIR}/shared)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)

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

target_link_libraries(qml_tests PRIVATE glean lottie nebula translations)

target_compile_definitions(qml_tests PRIVATE QUICK_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
target_compile_definitions(qml_tests PRIVATE UNIT_TEST)
target_compile_definitions(qml_tests PRIVATE MVPN_DUMMY)

# VPN Client source files
target_sources(qml_tests PRIVATE
    ${MVPN_SOURCE_DIR}/apps/vpn/constants.h
    ${MVPN_SOURCE_DIR}/apps/vpn/controller.h
    ${MVPN_SOURCE_DIR}/apps/vpn/cryptosettings.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/cryptosettings.h
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/env.h
    ${MVPN_SOURCE_DIR}/apps/vpn/externalophandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/externalophandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/filterproxymodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/filterproxymodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/inspector/inspectorhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feature.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/feature.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/featuremodel.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/featuremodel.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/server.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/server.h
    ${MVPN_SOURCE_DIR}/apps/vpn/models/subscriptiondata.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/models/subscriptiondata.h
    ${MVPN_SOURCE_DIR}/apps/vpn/mozillavpn.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkmanager.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/networkmanager.h
    ${MVPN_SOURCE_DIR}/apps/vpn/networkrequest.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/networkrequest.h
    ${MVPN_SOURCE_DIR}/apps/vpn/settingsholder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/settingsholder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/theme.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/theme.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.h
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummycryptosettings.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/qmlengineholder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/qmlengineholder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.h
    ${MVPN_SOURCE_DIR}/shared/hawkauth.cpp
    ${MVPN_SOURCE_DIR}/shared/hawkauth.h
    ${MVPN_SOURCE_DIR}/shared/hkdf.cpp
    ${MVPN_SOURCE_DIR}/shared/hkdf.h
    ${MVPN_SOURCE_DIR}/shared/ipaddress.cpp
    ${MVPN_SOURCE_DIR}/shared/ipaddress.h
    ${MVPN_SOURCE_DIR}/shared/logger.cpp
    ${MVPN_SOURCE_DIR}/shared/logger.h
    ${MVPN_SOURCE_DIR}/shared/loghandler.cpp
    ${MVPN_SOURCE_DIR}/shared/loghandler.h
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
)

# VPN Client UI resources
target_sources(qml_tests PRIVATE
    ${MVPN_SOURCE_DIR}/apps/vpn/ui/ui.qrc
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
  URI Mozilla.VPN.qmlcomponents
  VERSION 1.0
)
