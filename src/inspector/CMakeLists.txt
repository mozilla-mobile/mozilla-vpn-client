
add_library(inspector INTERFACE)

target_sources(inspector INTERFACE
    inspector.cpp
    inspector.h
    inspectorcommandhandler.cpp
    inspectorcommandhandler.h
    inspectorhotreloader.cpp
    inspectorhotreloader.h
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
)