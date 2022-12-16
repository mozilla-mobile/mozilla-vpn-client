# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MVPN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${MVPN_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR}/apps/vpn)
include_directories(${MVPN_SOURCE_DIR}/shared)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin)
include_directories(${MVPN_SOURCE_DIR}/shared/hacl-star/kremlin/minimal)

qt_add_executable(auth_tests EXCLUDE_FROM_ALL)
set_target_properties(auth_tests PROPERTIES FOLDER "Tests")
add_dependencies(build_tests auth_tests)

target_link_libraries(auth_tests PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Gui
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
)
target_link_libraries(auth_tests PRIVATE glean translations)

target_compile_definitions(auth_tests PRIVATE UNIT_TEST)
target_compile_definitions(auth_tests PRIVATE MVPN_DEBUG)
target_compile_definitions(auth_tests PRIVATE MVPN_DUMMY)

# VPN Client source files
target_sources(auth_tests PRIVATE
    ${MVPN_SOURCE_DIR}/apps/vpn/appconstants.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/appconstants.h
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinapp.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinapp.h
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinapplistener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinapplistener.h
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinappsession.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/authenticationinappsession.h
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/incrementaldecoder.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationinapp/incrementaldecoder.h
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationlistener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/authenticationlistener.h
    ${MVPN_SOURCE_DIR}/apps/vpn/controller.h
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/dnspingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/errorhandler.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/errorhandler.h
    ${MVPN_SOURCE_DIR}/apps/vpn/inspector/inspectorhandler.h
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
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pinghelper.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/pingsenderfactory.h
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/platforms/dummy/dummypingsender.h
    ${MVPN_SOURCE_DIR}/apps/vpn/simplenetworkmanager.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/simplenetworkmanager.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/authenticate/desktopauthenticationlistener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/authenticate/desktopauthenticationlistener.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/authenticate/taskauthenticate.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/authenticate/taskauthenticate.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/deleteaccount/taskdeleteaccount.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/deleteaccount/taskdeleteaccount.h
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/function/taskfunction.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/tasks/function/taskfunction.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/updater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/versionapi.h
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/update/webupdater.h
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.cpp
    ${MVPN_SOURCE_DIR}/apps/vpn/urlopener.h
    ${MVPN_SOURCE_DIR}/shared/constants.cpp
    ${MVPN_SOURCE_DIR}/shared/constants.h
    ${MVPN_SOURCE_DIR}/shared/cryptosettings.cpp
    ${MVPN_SOURCE_DIR}/shared/cryptosettings.h
    ${MVPN_SOURCE_DIR}/shared/env.h
    ${MVPN_SOURCE_DIR}/shared/feature.cpp
    ${MVPN_SOURCE_DIR}/shared/feature.h
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Chacha20Poly1305_32.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Curve25519_51.c
    ${MVPN_SOURCE_DIR}/shared/hacl-star/Hacl_Poly1305_32.c
    ${MVPN_SOURCE_DIR}/shared/hawkauth.cpp
    ${MVPN_SOURCE_DIR}/shared/hawkauth.h
    ${MVPN_SOURCE_DIR}/shared/hkdf.cpp
    ${MVPN_SOURCE_DIR}/shared/hkdf.h
    ${MVPN_SOURCE_DIR}/shared/ipaddress.cpp
    ${MVPN_SOURCE_DIR}/shared/ipaddress.h
    ${MVPN_SOURCE_DIR}/shared/leakdetector.cpp
    ${MVPN_SOURCE_DIR}/shared/leakdetector.h
    ${MVPN_SOURCE_DIR}/shared/logger.cpp
    ${MVPN_SOURCE_DIR}/shared/logger.h
    ${MVPN_SOURCE_DIR}/shared/loghandler.cpp
    ${MVPN_SOURCE_DIR}/shared/loghandler.h
    ${MVPN_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc1918.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc1918.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4193.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4193.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4291.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc4291.h
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc5735.cpp
    ${MVPN_SOURCE_DIR}/shared/rfc/rfc5735.h
    ${MVPN_SOURCE_DIR}/shared/settingsholder.cpp
    ${MVPN_SOURCE_DIR}/shared/settingsholder.h
    ${MVPN_SOURCE_DIR}/shared/task.h
    ${MVPN_SOURCE_DIR}/shared/taskscheduler.cpp
    ${MVPN_SOURCE_DIR}/shared/taskscheduler.h
    ${MVPN_SOURCE_DIR}/shared/versionutils.cpp
    ${MVPN_SOURCE_DIR}/shared/versionutils.h
)

# Generate the version header
configure_file(${MVPN_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_sources(auth_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Auth test mock resources
target_sources(auth_tests PRIVATE
    auth.qrc
)

# Auth test source files
target_sources(auth_tests PRIVATE
    main.cpp
    version.h
    testemailvalidation.cpp
    testemailvalidation.h
    testpasswordvalidation.cpp
    testpasswordvalidation.h
    testsignupandin.cpp
    testsignupandin.h
    mocmozillavpn.cpp
    ../qml/moccontroller.cpp
    ../unit/mocinspectorhandler.cpp
)

## Add the tests to be run.
add_test(NAME auth_tests COMMAND auth_tests)
