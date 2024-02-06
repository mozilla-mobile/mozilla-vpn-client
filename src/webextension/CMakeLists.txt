# Sources for desktop platforms.
if(CMAKE_CROSSCOMPILING)
    return()
endif()

target_sources(mozillavpn-sources INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/baseadapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/baseadapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/vpnadapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webextension/vpnadapter.cpp
)
 

target_compile_definitions(mozillavpn PUBLIC MVPN_WEBEXTENSION)

