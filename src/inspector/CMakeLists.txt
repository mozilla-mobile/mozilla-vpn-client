
#add_library(inspector INTERFACE)

target_sources(mozillavpn-sources INTERFACE
    inspector.cpp
    inspector.h
    inspectoritempicker.cpp
    inspectoritempicker.h

    tools/qquickinspector.cpp
    tools/qquickinspector.h
    tools/qqitemwrapper.h
    tools/qqitemwrapper.cpp
    tools/inspectorcommandhandler.cpp
    tools/inspectorcommandhandler.h
    tools/hotreloader.cpp
    tools/hotreloader.h

    server/factory.cpp
    server/factory.h
    server/websockettransport.cpp
    server/websockettransport.h
    server/websocketserver.cpp
    server/websocketserver.h
)
