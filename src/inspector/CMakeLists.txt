
add_library(inspector INTERFACE)

target_sources(inspector INTERFACE
    inspector.cpp
    inspector.h
    inspectorcommandhandler.cpp
    inspectorcommandhandler.h
    inspectoritempicker.cpp
    inspectoritempicker.h
    inspectorutils.cpp
    inspectorutils.h


    tools/qquickinspector.cpp
    tools/qquickinspector.h
    tools/qqitemwrapper.h
    tools/qqitemwrapper.cpp
    tools/hotreloader.cpp
    tools/hotreloader.h

    server/factory.cpp
    server/factory.h
    server/websockettransport.cpp
    server/websockettransport.h
    server/websocketserver.cpp
    server/websocketserver.h
)
