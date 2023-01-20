# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This CMAKE File will Integrate Sentry into MVPN


# Defines which OS builds can include sentry. Check src/cmake Lists for all values of MZ_PLATFORM_NAME
set(SENTRY_SUPPORTED_OS  "Windows" "Darwin" "Android")
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include(ExternalProject)



LIST(FIND SENTRY_SUPPORTED_OS ${CMAKE_SYSTEM_NAME} _SUPPORTED)

if(NOT SENTRY_DSN OR NOT SENTRY_ENVELOPE_ENDPOINT)
message( "Disabling Sentry, as params are not given")
set( _SUPPORTED -1)
endif()



## Remove support for android 32bit.
## It's  currently broken. see: VPN-3332
if( CMAKE_ANDROID_ARCH STREQUAL "x86" )
    set( _SUPPORTED -1)
elseif( CMAKE_ANDROID_ARCH STREQUAL "arm" )
    set( _SUPPORTED -1)
endif()

if( ${_SUPPORTED} GREATER -1 )
    message("Building sentry for ${CMAKE_SYSTEM_NAME}")
    target_compile_definitions(mozillavpn PRIVATE SENTRY_ENVELOPE_ENDPOINT="${SENTRY_ENVELOPE_ENDPOINT}")
    target_compile_definitions(mozillavpn PRIVATE SENTRY_DSN="${SENTRY_DSN}")
    target_compile_definitions(mozillavpn PRIVATE SENTRY_ENABLED)
    # Sentry support is given
    target_sources(mozillavpn PRIVATE
        apps/vpn/sentry/sentryadapter.cpp
        apps/vpn/sentry/sentryadapter.h     
        apps/vpn/tasks/sentry/tasksentry.cpp
        apps/vpn/tasks/sentry/tasksentry.h
    )

    # Configure Linking and Compile
    if(APPLE)
        include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
        # Let sentry.h know we are using a static build
        target_compile_definitions(mozillavpn PRIVATE SENTRY_BUILD_STATIC)
        # Let mozilla-vpn know we need to provide the upload client
        target_compile_definitions(mozillavpn PRIVATE SENTRY_NONE_TRANSPORT)
        # Compile Static for apple and link to libsentry.a
        target_link_libraries(mozillavpn PUBLIC libsentry.a)
        target_link_libraries(mozillavpn PUBLIC breakpad_client.a)
        # We are using breakpad as a backend - in process stackwalking is never the best option ... however!
        # this is super easy to link against and we do not need another binary shipped with the client.
        SET(SENTRY_ARGS -DSENTRY_BACKEND=breakpad -DSENTRY_BUILD_SHARED_LIBS=false -DSENTRY_TRANSPORT=none -DSENTRY_BUILD_TESTS=off -DSENTRY_BUILD_EXAMPLES=off)
    endif()
    if(WIN32)
        # Let sentry.h know we are using a static build
        target_compile_definitions(mozillavpn PRIVATE SENTRY_BUILD_STATIC)
        # Link against static sentry + breakpad + the stack unwind utils
        target_link_libraries(mozillavpn PUBLIC sentry.lib)
        target_link_libraries(mozillavpn PUBLIC breakpad_client.lib)
        target_link_libraries(mozillavpn PUBLIC dbghelp.lib)
        # Windows will use the winhttp transport btw
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=false -DSENTRY_BACKEND=breakpad -DCMAKE_BUILD_TYPE=Release)
    endif()

    if(ANDROID)
        # Let mozilla-vpn know we need to provide the upload client
        target_compile_definitions(mozillavpn PRIVATE SENTRY_NONE_TRANSPORT)

        target_link_libraries(mozillavpn PUBLIC libsentry.a)
        target_link_libraries(mozillavpn PUBLIC libunwindstack.a)
        # We can only use inproc as crash backend.
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=false
                        -DANDROID_PLATFORM=21
                        -DCMAKE_SYSTEM_NAME=Android
                        -DANDROID_ABI=${ANDROID_ABI}
                        -DCMAKE_ANDROID_NDK=${ANDROID_NDK_ROOT}
                        -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake
                        -DSENTRY_BACKEND=inproc
            )
    endif()

    include(ExternalProject)
    ExternalProject_Add(sentry
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../3rdparty/sentry
        GIT_REPOSITORY https://github.com/getsentry/sentry-native/
        GIT_TAG 0.5.0
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} ${SENTRY_ARGS}
    )

    target_include_directories(mozillavpn PUBLIC ${EXTERNAL_INSTALL_LOCATION}/include)
    target_link_directories( mozillavpn PUBLIC ${EXTERNAL_INSTALL_LOCATION}/lib)
    add_dependencies(mozillavpn sentry)
else()
    # Sentry is not supported on this Plattform.
    message("Sentry supported OS -> ${SENTRY_SUPPORTED_OS}")
    message("Cannot build sentry for ${CMAKE_SYSTEM_NAME}")
endif()

