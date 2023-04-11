# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This CMAKE File will Integrate Sentry into MZ


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
    target_compile_definitions(shared-sources INTERFACE SENTRY_ENVELOPE_ENDPOINT="${SENTRY_ENVELOPE_ENDPOINT}")
    target_compile_definitions(shared-sources INTERFACE SENTRY_DSN="${SENTRY_DSN}")
    target_compile_definitions(shared-sources INTERFACE SENTRY_ENABLED)
    # Let's the app know we need to provide the upload client
    target_compile_definitions(shared-sources INTERFACE SENTRY_NONE_TRANSPORT)

    # Sentry support is given
    target_sources(shared-sources INTERFACE
        shared/sentry/sentryadapter.cpp
        shared/sentry/sentryadapter.h
        shared/tasks/sentry/tasksentry.cpp
        shared/tasks/sentry/tasksentry.h
    )

    # Configure Linking and Compile
    if(APPLE)
        include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
        # Let sentry.h know we are using a static build
        target_compile_definitions(shared-sources INTERFACE SENTRY_BUILD_STATIC)
        # Compile Static for apple and link to libsentry.a
        target_link_libraries(shared-sources INTERFACE libsentry.a)
        target_link_libraries(shared-sources INTERFACE breakpad_client.a)
        # We are using breakpad as a backend - in process stackwalking is never the best option ... however!
        # this is super easy to link against and we do not need another binary shipped with the client.
        SET(SENTRY_ARGS -DSENTRY_BACKEND=breakpad -DSENTRY_BUILD_SHARED_LIBS=false -DSENTRY_TRANSPORT=none -DSENTRY_BUILD_TESTS=off -DSENTRY_BUILD_EXAMPLES=off)
        if(CMAKE_OSX_ARCHITECTURES)
            STRING(REPLACE ";" "$<SEMICOLON>" OSX_ARCH_LISTSAFE "${CMAKE_OSX_ARCHITECTURES}")
            LIST(APPEND SENTRY_ARGS -DCMAKE_OSX_ARCHITECTURES:STRING=${OSX_ARCH_LISTSAFE})
        endif()
    endif()
    if(WIN32)
        # Let sentry.h know we are using a static build
        target_compile_definitions(shared-sources INTERFACE SENTRY_BUILD_STATIC)
        # Link against static sentry + breakpad + the stack unwind utils
        target_link_libraries(shared-sources INTERFACE sentry.lib)
        target_link_libraries(shared-sources INTERFACE breakpad_client.lib)
        target_link_libraries(shared-sources INTERFACE dbghelp.lib)
        SET(SENTRY_ARGS -DSENTRY_BUILD_SHARED_LIBS=false -DSENTRY_BACKEND=breakpad -DSENTRY_TRANSPORT=none -DCMAKE_BUILD_TYPE=Release)
    endif()

    if(ANDROID)
        target_link_libraries(shared-sources INTERFACE libsentry.a)
        target_link_libraries(shared-sources INTERFACE libunwindstack.a)
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
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/sentry
        GIT_SUBMODULES 3rdparty/sentry
        GIT_SUBMODULES_RECURSE true
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} ${SENTRY_ARGS}
    )

    target_include_directories(shared-sources INTERFACE ${EXTERNAL_INSTALL_LOCATION}/include)
    target_link_directories( shared-sources INTERFACE ${EXTERNAL_INSTALL_LOCATION}/lib)
    add_dependencies(shared-sources sentry)
else()
    # Sentry is not supported on this Plattform.
    message("Sentry supported OS -> ${SENTRY_SUPPORTED_OS}")
    message("Cannot build sentry for ${CMAKE_SYSTEM_NAME}")
endif()

