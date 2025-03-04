# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


target_sources(shared-sources INTERFACE
        inspectorhandler.cpp
        inspectorhandler.h
        inspectorhotreloader.h
        inspectorhotreloader.cpp
        inspectoritempicker.h
        inspectoritempicker.cpp
        inspectorutils.h
        inspectorutils.cpp   
)

if(${MZ_PLATFORM_NAME} STREQUAL "wasm")
    target_sources(shared-sources INTERFACE
        wasminspector.cpp
        wasminspector.h
    )
    else()
    target_sources(shared-sources INTERFACE
        inspectorwebsocketconnection.cpp
        inspectorwebsocketconnection.h
        inspectorwebsocketserver.h
        inspectorwebsocketserver.cpp    
    )
endif()
