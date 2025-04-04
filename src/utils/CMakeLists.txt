# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mzutils STATIC
    interfaceconfig.cpp
    interfaceconfig.h
    ipaddress.cpp
    ipaddress.h
    leakdetector.cpp
    leakdetector.h
    logger.cpp
    logger.h
    loghandler.cpp
    loghandler.h
    loglevel.h
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(mzutils PRIVATE
        signalhandler.cpp
        signalhandler.h
     )
endif()

target_include_directories(mzutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mzutils PUBLIC Qt6::Core Qt6::Network)

target_compile_definitions(mzutils PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_compile_definitions(mzutils PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")
