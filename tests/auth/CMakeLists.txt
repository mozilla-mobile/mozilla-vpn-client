# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DUNIT_TEST)
add_definitions(-DMVPN_DEBUG)
add_definitions(-DMVPN_DUMMY)

get_filename_component(MVPN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src ABSOLUTE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${MVPN_SOURCE_DIR})
include_directories(${MVPN_SOURCE_DIR}/hacl-star)
include_directories(${MVPN_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${MVPN_SOURCE_DIR}/hacl-star/kremlin/minimal)

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

# VPN Client source files
target_sources(auth_tests PRIVATE
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinapp.cpp
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinapp.h
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.cpp
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinapplistener.h
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinappsession.cpp
    ${MVPN_SOURCE_DIR}/authenticationinapp/authenticationinappsession.h
    ${MVPN_SOURCE_DIR}/authenticationinapp/incrementaldecoder.cpp
    ${MVPN_SOURCE_DIR}/authenticationinapp/incrementaldecoder.h
    ${MVPN_SOURCE_DIR}/authenticationlistener.cpp
    ${MVPN_SOURCE_DIR}/authenticationlistener.h
    ${MVPN_SOURCE_DIR}/constants.cpp
    ${MVPN_SOURCE_DIR}/constants.h
    ${MVPN_SOURCE_DIR}/errorhandler.cpp
    ${MVPN_SOURCE_DIR}/errorhandler.h
    ${MVPN_SOURCE_DIR}/hawkauth.cpp
    ${MVPN_SOURCE_DIR}/hawkauth.h
    ${MVPN_SOURCE_DIR}/hkdf.cpp
    ${MVPN_SOURCE_DIR}/hkdf.h
    ${MVPN_SOURCE_DIR}/inspector/inspectorhandler.h
    ${MVPN_SOURCE_DIR}/ipaddress.cpp
    ${MVPN_SOURCE_DIR}/ipaddress.h
    ${MVPN_SOURCE_DIR}/leakdetector.cpp
    ${MVPN_SOURCE_DIR}/leakdetector.h
    ${MVPN_SOURCE_DIR}/logger.cpp
    ${MVPN_SOURCE_DIR}/logger.h
    ${MVPN_SOURCE_DIR}/loghandler.cpp
    ${MVPN_SOURCE_DIR}/loghandler.h
    ${MVPN_SOURCE_DIR}/models/feature.cpp
    ${MVPN_SOURCE_DIR}/models/feature.h
    ${MVPN_SOURCE_DIR}/models/featuremodel.cpp
    ${MVPN_SOURCE_DIR}/models/featuremodel.h
    ${MVPN_SOURCE_DIR}/models/server.cpp
    ${MVPN_SOURCE_DIR}/models/server.h
    ${MVPN_SOURCE_DIR}/mozillavpn.h
    ${MVPN_SOURCE_DIR}/networkmanager.cpp
    ${MVPN_SOURCE_DIR}/networkmanager.h
    ${MVPN_SOURCE_DIR}/networkrequest.cpp
    ${MVPN_SOURCE_DIR}/networkrequest.h
    ${MVPN_SOURCE_DIR}/rfc/rfc1918.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc1918.h
    ${MVPN_SOURCE_DIR}/rfc/rfc4193.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc4193.h
    ${MVPN_SOURCE_DIR}/rfc/rfc4291.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc4291.h
    ${MVPN_SOURCE_DIR}/rfc/rfc5735.cpp
    ${MVPN_SOURCE_DIR}/rfc/rfc5735.h
    ${MVPN_SOURCE_DIR}/settingsholder.cpp
    ${MVPN_SOURCE_DIR}/settingsholder.h
    ${MVPN_SOURCE_DIR}/simplenetworkmanager.cpp
    ${MVPN_SOURCE_DIR}/simplenetworkmanager.h
    ${MVPN_SOURCE_DIR}/task.h
    ${MVPN_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.cpp
    ${MVPN_SOURCE_DIR}/tasks/authenticate/desktopauthenticationlistener.h
    ${MVPN_SOURCE_DIR}/tasks/authenticate/taskauthenticate.cpp
    ${MVPN_SOURCE_DIR}/tasks/authenticate/taskauthenticate.h
    ${MVPN_SOURCE_DIR}/tasks/deleteaccount/taskdeleteaccount.cpp
    ${MVPN_SOURCE_DIR}/tasks/deleteaccount/taskdeleteaccount.h
    ${MVPN_SOURCE_DIR}/tasks/function/taskfunction.cpp
    ${MVPN_SOURCE_DIR}/tasks/function/taskfunction.h
    ${MVPN_SOURCE_DIR}/taskscheduler.cpp
    ${MVPN_SOURCE_DIR}/taskscheduler.h
    ${MVPN_SOURCE_DIR}/update/updater.cpp
    ${MVPN_SOURCE_DIR}/update/updater.h
    ${MVPN_SOURCE_DIR}/update/versionapi.cpp
    ${MVPN_SOURCE_DIR}/update/versionapi.h
    ${MVPN_SOURCE_DIR}/update/webupdater.cpp
    ${MVPN_SOURCE_DIR}/update/webupdater.h
    ${MVPN_SOURCE_DIR}/urlopener.cpp
    ${MVPN_SOURCE_DIR}/urlopener.h
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
    ../unit/mocinspectorhandler.cpp
)

## Add the tests to be run.
add_test(NAME auth_tests COMMAND auth_tests)
