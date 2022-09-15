# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This CMAKE File will Integrate Sentry into MVPN


# Defines which OS builds can include sentry. Check src/cmake Lists for all values of MVPN_PLATFORM_NAME
set(SENTRY_SUPPORTED_OS  "Windows" "Darwin")
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include(ExternalProject)
 


LIST(FIND SENTRY_SUPPORTED_OS ${CMAKE_SYSTEM_NAME} _SUPPORTED)

if( ${_SUPPORTED} GREATER -1 )
    message("Building sentry for ${CMAKE_SYSTEM_NAME}")
    target_compile_definitions(mozillavpn PRIVATE SENTRY_ENABLED)
    # Sentry support is given
    target_sources(mozillavpn PRIVATE
    sentry/sentryadapter.cpp
    sentry/sentryadapter.h
    )

    # Configure Linking and Compile
    if(APPLE)
        # Compile Static for apple and link to libsentry.a
        target_link_libraries(mozillavpn PUBLIC libsentry.a)
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=false)
        osx_bundle_files(mozillavpn
            FILES ${CMAKE_BINARY_DIR}/external/bin/crashpad_handler
            DESTINATION MacOS
        )
    endif()
    if(WIN32)
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=false  -D SENTRY_BACKEND=breakpad)
        # Link against static sentry + breakpad + the stack unwind utils
        target_link_libraries(mozillavpn PUBLIC sentry.lib)
        target_link_libraries(mozillavpn PUBLIC breakpad_client.lib)
        target_link_libraries(mozillavpn PUBLIC dbghelp.lib)
        # Make sure the sentry header does not try to dll-link it :) 
        target_compile_definitions(mozillavpn PRIVATE SENTRY_BUILD_STATIC)

    endif()

    include(ExternalProject)
    ExternalProject_Add(sentry
        GIT_REPOSITORY https://github.com/getsentry/sentry-native/
        GIT_TAG 0.5.0
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} ${SENTRY_ARGS}
    )

    target_include_directories(mozillavpn PUBLIC ${EXTERNAL_INSTALL_LOCATION}/include)
    target_link_directories( mozillavpn PUBLIC ${EXTERNAL_INSTALL_LOCATION}/lib)
    add_dependencies(mozillavpn sentry)
else()
    message("Sentry supported OS -> ${SENTRY_SUPPORTED_OS}")
    message("Skipping building sentry for ${CMAKE_SYSTEM_NAME}")
    # Sentry is not supported on this Plattform, let's
    # only include a dummy client :) 
    target_sources(mozillavpn PRIVATE
       sentry/dummysentryadapter.cpp
       sentry/sentryadapter.h
    )
endif()