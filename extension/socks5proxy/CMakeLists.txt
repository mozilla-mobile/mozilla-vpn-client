# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# No need to compile this for mobile/web
if(CMAKE_CROSSCOMPILING)
    return()
endif()

if(CMAKE_OSX_ARCHITECTURES)
    STRING(REPLACE ";" "$<SEMICOLON>" C_ARES_OSX_ARCH_LISTSAFE "${CMAKE_OSX_ARCHITECTURES}")
    LIST(APPEND SENTRY_ARGS -DCMAKE_OSX_ARCHITECTURES:STRING=${C_ARES_OSX_ARCH_LISTSAFE})
endif()


include(ExternalProject)
ExternalProject_Add(c-ares
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/c-ares
        GIT_SUBMODULES 3rdparty/c-ares
        GIT_SUBMODULES_RECURSE true
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCARES_STATIC=ON
            -DCARES_SHARED=OFF
            -DCARES_SHARED=OFF
            -DCARES_BUILD_TESTS=OFF
            -DCARES_BUILD_CONTAINER_TESTS=OFF
            -DCARES_BUILD_TOOLS=OFF
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
            -DCMAKE_OSX_ARCHITECTURES:STRING=${C_ARES_OSX_ARCH_LISTSAFE}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_STATIC_LIBRARY_PREFIX=""
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external
            
)

add_subdirectory(src)
add_subdirectory(bin)
add_subdirectory(tests)

target_compile_definitions(shared-sources INTERFACE MZ_PROXY_ENABLED)



