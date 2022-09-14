# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This CMAKE File will Integrate Sentry into MVPN


# Defines which OS builds can include sentry. Check src/cmake Lists for all values of MVPN_PLATFORM_NAME
set(SENTRY_SUPPORTED_OS  windows macos)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include(ExternalProject)
 


LIST(FIND SENTRY_SUPPORTED_OS MVPN_PLATFORM_NAME IS_SUPPORTED)

if( IS_SUPPORTED==-1 )
    # Sentry is not supported on this Plattform, let's
    # only include a dummy client :) 
    target_sources(mozillavpn PRIVATE
       sentry/moc_sentryadapter.cpp
       sentry/sentryadapter.h
    )

else()
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
        # Compile a dll for windows, as some breakpad imports will fail otherwise >:c
        target_link_libraries(mozillavpn PUBLIC sentry.lib)
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=true)
        install(FILES ${EXTERNAL_INSTALL_LOCATION}/bin/sentry.dll DESTINATION .)
        install(FILES ${EXTERNAL_INSTALL_LOCATION}/bin/crashpad_handler DESTINATION .)

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
endif()


    #target_compile_definitions(mozillavpn PRIVATE SENTRY_BUILD_STATIC)

    #target_link_libraries(mozillavpn PUBLIC crashpad_compat.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_getopt.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_handler_lib.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_minidump.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_snapshot.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_util.lib)
    #target_link_libraries(mozillavpn PUBLIC crashpad_zlib.lib)