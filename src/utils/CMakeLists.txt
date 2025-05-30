# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mzutils STATIC
    chacha20poly1305.cpp
    chacha20poly1305.h
    collator.cpp
    collator.h
    curve25519.cpp
    curve25519.h
    hacl-star/Hacl_Chacha20.c
    hacl-star/Hacl_Chacha20Poly1305_32.c
    hacl-star/Hacl_Curve25519_51.c
    hacl-star/Hacl_Poly1305_32.c
    hawkauth.cpp
    hawkauth.h
    hkdf.cpp
    hkdf.h
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
    models/apierror.cpp
    models/apierror.h
    models/keys.cpp
    models/keys.h
    models/location.cpp
    models/location.h
    pingsender/dnspingsender.cpp
    pingsender/dnspingsender.h
    pingsender/dummypingsender.cpp
    pingsender/dummypingsender.h
    pingsender/pingsender.cpp
    pingsender/pingsender.h
    pingsender/tcppingsender.cpp
    pingsender/tcppingsender.h
    rfc/rfc1112.cpp
    rfc/rfc1112.h
    rfc/rfc1918.cpp
    rfc/rfc1918.h
    rfc/rfc4193.cpp
    rfc/rfc4193.h
    rfc/rfc4291.cpp
    rfc/rfc4291.h
    rfc/rfc5735.cpp
    rfc/rfc5735.h
    task.h
    taskfunction.cpp
    taskfunction.h
    taskgroup.cpp
    taskgroup.h
    taskscheduler.cpp
    taskscheduler.h
)

# Signal handling for unix platforms
if(UNIX)
     target_sources(mzutils PRIVATE
        signalhandler.cpp
        signalhandler.h
     )
endif()

target_link_libraries(mzutils PUBLIC Qt6::Core Qt6::Network)

target_include_directories(mzutils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pingsender
)
target_include_directories(mzutils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star
    ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin
    ${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal
)

target_compile_definitions(mzutils PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_compile_definitions(mzutils PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")

add_subdirectory(tests)
