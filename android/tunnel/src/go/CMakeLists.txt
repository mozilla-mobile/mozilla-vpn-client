cmake_minimum_required(VERSION 3.18.1)
project(tunnel-go LANGUAGES C) # Important: set a valid language

# Find the Go executable
find_program(GO_EXECUTABLE go REQUIRED)
if(NOT GO_EXECUTABLE)
    message(FATAL_ERROR "Go compiler not found!")
endif()



# Set variables depending on Android ABI
if(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(GOARCH "arm")
    set(GOARM "7")
    set(GO_TRIPLE "armv7a-linux-androideabi")
elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(GOARCH "arm64")
    set(GO_TRIPLE "aarch64-linux-android")
elseif(ANDROID_ABI STREQUAL "x86")
    set(GOARCH "386")
    set(GO_TRIPLE "i686-linux-android")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(GOARCH "amd64")
    set(GO_TRIPLE "x86_64-linux-android")
else()
    message(FATAL_ERROR "Unsupported Android ABI: ${ANDROID_ABI}")
endif()

# Android API level
set(ANDROID_API_LEVEL 21)

# Path to the NDK toolchain
set(NDK_TOOLCHAIN "${ANDROID_NDK}/toolchains/llvm/prebuilt/$<LOWER_CASE:${CMAKE_HOST_SYSTEM_NAME}>-x86_64")

# Set paths explicitly for CGO
set(CGO_CFLAGS "--sysroot=${NDK_TOOLCHAIN}/sysroot -I${NDK_TOOLCHAIN}/sysroot/usr/include")
set(CGO_LDFLAGS "--sysroot=${NDK_TOOLCHAIN}/sysroot -L${NDK_TOOLCHAIN}/sysroot/usr/lib/${GO_TRIPLE}/${ANDROID_API_LEVEL} -llog")

# Go build environment
set(GO_ENV
        "CGO_ENABLED=1"
        "GOOS=android"
        "GOARCH=${GOARCH}"
        "CC=${NDK_TOOLCHAIN}/bin/${GO_TRIPLE}${ANDROID_API_LEVEL}-clang"
        "CGO_CFLAGS=${CGO_CFLAGS}"
        "CGO_LDFLAGS=${CGO_LDFLAGS}"
)

# Set Go source path and output location
set(GO_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(OUTPUT_LIB_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ANDROID_ABI}")
file(MAKE_DIRECTORY "${OUTPUT_LIB_DIR}")

# Custom command for building Go shared lib
add_custom_command(
        OUTPUT "${OUTPUT_LIB_DIR}/libtunnel.so"
        COMMAND ${CMAKE_COMMAND} -E env ${GO_ENV}
        ${GO_EXECUTABLE} build -buildmode=c-shared
        -o "${OUTPUT_LIB_DIR}/libtunnel.so"
        "${GO_SRC}"
        WORKING_DIRECTORY "${GO_SRC}"
        COMMENT "Building Go shared library for ${ANDROID_ABI}"
        VERBATIM
)
# Depend on the custom Go build command
add_custom_target(go_shared_lib ALL DEPENDS "${OUTPUT_LIB_DIR}/libtunnel.so")

add_library(libtunnel SHARED IMPORTED)
set_target_properties(libtunnel PROPERTIES IMPORTED_LOCATION "${OUTPUT_LIB_DIR}/libtunnel.so")
add_dependencies(libtunnel go_shared_lib)


# Gradle cannot comprehend the idea of someone using another language that is not
# c/c++ in cmake
# So if we define only a go-target it freaks out.
# Therefore in order to make it happy we will compile this empty.c file and have it deleted
# right after, what a lovely build tool.
add_library(gradleSacarficialLamb SHARED EXCLUDE_FROM_ALL dummy.c)
add_custom_command(TARGET gradleSacarficialLamb POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove "$<TARGET_FILE:gradleSacarficialLamb>"
        COMMENT "Removing placeholder library after build"
)

# Post-build: Copy libtunnel.so to jniLibs for AAR packaging
set(JNI_LIBS_DIR "${CMAKE_SOURCE_DIR}/../../src/main/jniLibs/${ANDROID_ABI}")
file(MAKE_DIRECTORY "${JNI_LIBS_DIR}")
add_custom_command(TARGET go_shared_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${OUTPUT_LIB_DIR}/libtunnel.so" "${JNI_LIBS_DIR}/libtunnel.so"
    COMMENT "Copying libtunnel.so to jniLibs for AAR packaging"
)

set_target_properties(gradleSacarficialLamb PROPERTIES
    LINKER_LANGUAGE C
    INTERFACE_LINK_LIBRARIES libtunnel
)
