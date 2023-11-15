
add_library(inspector INTERFACE)

target_sources(inspector INTERFACE
    inspector.cpp
    inspector.h
    inspectorcommandhandler.cpp
    inspectorcommandhandler.h
    inspectorserver.h
    inspectorserver.cpp
    inspectoritempicker.cpp
    inspectoritempicker.h
    inspectorutils.cpp
    inspectorutils.h
    inspectorwebsockettransport.cpp
    inspectorwebsockettransport.h
    inspectorwebsocketserver.cpp
    inspectorwebsocketserver.h

    tools/qquickinspector.cpp
    tools/qquickinspector.h
    tools/qqitemwrapper.h
    tools/qqitemwrapper.cpp
    tools/hotreloader.cpp
    tools/hotreloader.h
)