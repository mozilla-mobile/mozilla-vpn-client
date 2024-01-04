# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(inspector STATIC)

target_sources(inspector PUBLIC
    inspector.cpp
    inspector.h

    tools/qquickinspector.cpp
    tools/qquickinspector.h
    tools/qqitemwrapper.h
    tools/qqitemwrapper.cpp
    tools/qmlpath.h
    tools/qmlpath.cpp
    tools/commandhandler.cpp
    tools/commandhandler.h
    tools/command.h

    server/factory.cpp
    server/factory.h
    server/websockettransport.cpp
    server/websockettransport.h
    server/websocketserver.cpp
    server/websocketserver.h
)
find_package(Qt6 COMPONENTS
    Core
    Quick
    Network
    WebSockets
    WebChannel
)

target_link_libraries(inspector PUBLIC 
    Qt6::WebChannel
    Qt6::Core
    Qt6::WebSockets
    Qt6::Network
    Qt6::Quick
)

target_sources(mozillavpn-sources INTERFACE
    legacy/commands.h
    legacy/commands.cpp
)
