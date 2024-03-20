# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Sets Defaults like `-Wall -Werror` if we know it will not
# explode on that target + compiler
function(mz_target_handle_warnings MZ_TARGET)
    if(MSVC OR IOS)
        return()
    endif()
    # Just don't for wasm
    if( ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
        return()
    endif()
    # Check if the target is an interface lib
    get_target_property(target_type ${MZ_TARGET} TYPE)
    if (${target_type} STREQUAL "INTERFACE_LIBRARY")
        set(scope "INTERFACE")
    else()
        set(scope "PRIVATE")
    endif()

    target_compile_options( ${MZ_TARGET} ${scope} -Wall -Werror -Wno-conversion)
endfunction()

# MZ_ADD_NEW_MODULE: A utility function for adding a new module to this project.
#
# Targets added:
# - TARGET_NAME
# - <TARGET_NAME>-alltests -- Builds all tests for this target
# - <TARGET_NAME>-<TESTFILENAME> -- Builds one specific test
#
# Usage:
# MZ_ADD_NEW_MODULE(
#     TARGET_NAME <target_name>
#     [INCLUDE_DIRECTORIES <include_directories>]
#     [SOURCES <sources>]
#     [IOS_SOURCES <ios_sources>]
#     [ANDROID_SOURCES <android_sources>]
#     [MACOS_SOURCES <macos_sources>]
#     [LINUX_SOURCES <linux_sources>]
#     [WINDOWS_SOURCES <windows_sources>]
#     [WASM_SOURCES <wasm_sources>]
#     [DUMMY_SOURCES <dummy_sources>]
#     [TEST_SOURCES <test_sources>]
#     [QT_DEPENDENCIES <qt_dependencies>]
#     [MZ_DEPENDENCIES <mz_dependencies>]
#     [RUST_DEPENDENCIES <rust_dependencies>]
#     [EXTRA_DEPENDENCIES <extra_dependencies>]
#     [TEST_DEPENDENCIES <test_dependencies>]
#     [ANDROID_DEPENDENCIES <android_dependencies>]
#     [MACOS_DEPENDENCIES <macos_dependencies>]
#     [LINUX_DEPENDENCIES <linux_dependencies>]
#     [WINDOWS_DEPENDENCIES <windows_dependencies>]
#     [WASM_DEPENDENCIES <wasm_dependencies>]
#     [DUMMY_DEPENDENCIES <dummy_dependencies>]
# )
#
# Parameters:
# - TARGET_NAME: The name of the target module.
# - INCLUDE_DIRECTORIES: (Optional) List of additional include directories for the module.
# - SOURCES: (Optional) List of source files for the module.
# - IOS_SOURCES: (Optional) List of iOS-specific source files for the module.
# - ANDROID_SOURCES: (Optional) List of Android-specific source files for the module.
# - MACOS_SOURCES: (Optional) List of macOS-specific source files for the module.
# - LINUX_SOURCES: (Optional) List of Linux-specific source files for the module.
# - WINDOWS_SOURCES: (Optional) List of Windows-specific source files for the module.
# - WASM_SOURCES: (Optional) List of WebAssembly-specific source files for the module.
# - DUMMY_SOURCES: (Optional) List of dummy sources for the module.
# - TEST_SOURCES: (Optional) List of test source files for the module.
# - QT_DEPENDENCIES: (Optional) List of Qt dependencies for the module.
# - MZ_DEPENDENCIES: (Optional) List of custom module dependencies for the module.
# - RUST_DEPENDENCIES: (Optional) List of Rust dependencies for the module.
# - EXTRA_DEPENDENCIES: (Optional) List of additional dependencies for the module.
# - TEST_DEPENDENCIES: (Optional) List of test-only dependencies.
#   If the name of the dependency starts with `replace-<targetname>`
#   it will replace <targetname> dependency for tests.
# - IOS_DEPENDENCIES: (Optional) List of iOS-specific dependencies for the module.
# - ANDROID_DEPENDENCIES: (Optional) List of Android-specific dependencies for the module.
# - MACOS_DEPENDENCIES: (Optional) List of macOS-specific dependencies for the module.
# - LINUX_DEPENDENCIES: (Optional) List of Linux-specific dependencies for the module.
# - WINDOWS_DEPENDENCIES: (Optional) List of Windows-specific dependencies for the module.
# - WASM_DEPENDENCIES: (Optional) List of WebAssembly-specific dependencies for the module.
# - DUMMY_DEPENDENCIES: (Optional) List of dummy dependencies for the module.
#
# Example:
# mz_add_new_module(
#     TARGET_NAME mz_mymodule
#     INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
#     SOURCES src/file1.cpp src/file2.cpp
#     TEST_SOURCES tests/test_file.cpp
#     MZ_DEPENDENCIES mz_module
# )
function(mz_add_new_module)
    cmake_parse_arguments(
        MZ_ADD_NEW_MODULE # prefix
        "" # options
        "" # single-value args
        "TARGET_NAME;INCLUDE_DIRECTORIES;SOURCES;IOS_SOURCES;ANDROID_SOURCES;MACOS_SOURCES;LINUX_SOURCES;WINDOWS_SOURCES;WASM_SOURCES;DUMMY_SOURCES;TEST_SOURCES;QT_DEPENDENCIES;MZ_DEPENDENCIES;RUST_DEPENDENCIES;EXTRA_DEPENDENCIES;TEST_DEPENDENCIES;IOS_DEPENDENCIES;ANDROID_DEPENDENCIES;MACOS_DEPENDENCIES;LINUX_DEPENDENCIES;WINDOWS_DEPENDENCIES;WASM_DEPENDENCIES;DUMMY_DEPENDENCIES" # multi-value args
        ${ARGN})


    # Create a static lib for the new module.
    mz_add_library(
        NAME ${MZ_ADD_NEW_MODULE_TARGET_NAME}
        TYPE STATIC
    )

    # Generate sources lists
    mz_generate_sources_list(
        SOURCES ${MZ_ADD_NEW_MODULE_SOURCES}
        IOS_SOURCES ${MZ_ADD_NEW_MODULE_IOS_SOURCES}
        ANDROID_SOURCES ${MZ_ADD_NEW_MODULE_ANDROID_SOURCES}
        MACOS_SOURCES ${MZ_ADD_NEW_MODULE_MACOS_SOURCES}
        LINUX_SOURCES ${MZ_ADD_NEW_MODULE_LINUX_SOURCES}
        WINDOWS_SOURCES ${MZ_ADD_NEW_MODULE_WINDOWS_SOURCES}
        WASM_SOURCES ${MZ_ADD_NEW_MODULE_WASM_SOURCES}
        DUMMY_SOURCES ${MZ_ADD_NEW_MODULE_DUMMY_SOURCES}
    )
    target_sources(${MZ_ADD_NEW_MODULE_TARGET_NAME} PUBLIC ${ALL_SOURCES})

    # Generate dependencies lists
    mz_generate_link_libraries(
        QT_DEPENDENCIES  ${MZ_ADD_NEW_MODULE_QT_DEPENDENCIES}
        MZ_DEPENDENCIES ${MZ_ADD_NEW_MODULE_MZ_DEPENDENCIES}
        RUST_DEPENDENCIES ${MZ_ADD_NEW_MODULE_RUST_DEPENDENCIES}
        EXTRA_DEPENDENCIES ${MZ_ADD_NEW_MODULE_EXTRA_DEPENDENCIES}
        TEST_DEPENDENCIES ${MZ_ADD_NEW_MODULE_TEST_DEPENDENCIES}
        IOS_DEPENDENCIES ${MZ_ADD_NEW_MODULE_IOS_DEPENDENCIES}
        ANDROID_DEPENDENCIES ${MZ_ADD_NEW_MODULE_ANDROID_DEPENDENCIES}
        MACOS_DEPENDENCIES ${MZ_ADD_NEW_MODULE_MACOS_DEPENDENCIES}
        LINUX_DEPENDENCIES ${MZ_ADD_NEW_MODULE_LINUX_DEPENDENCIES}
        WINDOWS_DEPENDENCIES ${MZ_ADD_NEW_MODULE_WINDOWS_DEPENDENCIES}
        WASM_DEPENDENCIES ${MZ_ADD_NEW_MODULE_WASM_DEPENDENCIES}
        DUMMY_DEPENDENCIES ${MZ_ADD_NEW_MODULE_DUMMY_DEPENDENCIES}
    )
    target_link_libraries(${MZ_ADD_NEW_MODULE_TARGET_NAME} PUBLIC ${LINK_LIBRARIES})

    # Define include directories
    target_include_directories(${MZ_ADD_NEW_MODULE_TARGET_NAME} PUBLIC
        ${MZ_ADD_NEW_MODULE_INCLUDE_DIRECTORIES}
    )

    # Swift sources are added to a separate target.
    if(APPLE_SOURCES)
        mz_add_the_apple_stuff(
            TARGET_NAME
                ${MZ_ADD_NEW_MODULE_TARGET_NAME}
            SOURCES
                ${APPLE_SOURCES}
            DEPENDENCIES
                ${LINK_LIBRARIES}
            INCLUDE_DIRECTORIES
                ${MZ_ADD_NEW_MODULE_INCLUDE_DIRECTORIES}
        )
    endif()

    # Create separate targets for each test,
    # one target that builds all tests from this module
    # and finally add this module's tests to the build_tests target which builds all tests.
    if(MZ_ADD_NEW_MODULE_TEST_SOURCES)
        add_custom_target(${MZ_ADD_NEW_MODULE_TARGET_NAME}-alltests)

        add_dependencies(build_tests ${MZ_ADD_NEW_MODULE_TARGET_NAME}-alltests)

        set(CPP_TEST_FILES ${MZ_ADD_NEW_MODULE_TEST_SOURCES})
        list(FILTER CPP_TEST_FILES INCLUDE REGEX "\.cpp$")
        set(QRC_TEST_FILES ${MZ_ADD_NEW_MODULE_TEST_SOURCES})
        list(FILTER QRC_TEST_FILES INCLUDE REGEX "\.qrc$")

        foreach(TEST_FILE ${CPP_TEST_FILES})
            # The test executable name will be the name of the test file
            # + the name of the parent target as a prefix.
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            set(TEST_TARGET_NAME "${MZ_ADD_NEW_MODULE_TARGET_NAME}-${TEST_NAME}")

            mz_add_test_target(
                TARGET_NAME
                    ${TEST_TARGET_NAME}
                TEST_COMMAND
                    ${TEST_TARGET_NAME}
                PARENT_TARGET
                    ${MZ_ADD_NEW_MODULE_TARGET_NAME}
                SOURCES
                    ${TEST_FILE}
                    ${QRC_TEST_FILES}
                    ${ALL_SOURCES}
                DEPENDENCIES
                    ${TEST_LINK_LIBRARIES}
            )
            
            add_dependencies(${MZ_ADD_NEW_MODULE_TARGET_NAME}-alltests ${TEST_TARGET_NAME})
            
            # Check if the corresponding header file exists
            string(REGEX REPLACE "\.cpp$" ".h" HEADER_FILE ${TEST_FILE})
            if(EXISTS ${HEADER_FILE})
                # Add the header file to the executable if it exists.
                target_sources(${TEST_TARGET_NAME} PRIVATE ${TEST_FILE})
            endif()
        endforeach()
    endif()
    
    # Add Clang-tidy to that module
    mz_add_clang_tidy(${MZ_ADD_NEW_MODULE_TARGET_NAME})
endfunction()

function(mz_add_library)
    cmake_parse_arguments(
        MZ_ADD_LIBRARY # prefix
        "" # options
        "" # single-value args
        "NAME;TYPE" # multi-value args
        ${ARGN})

    add_library(${MZ_ADD_LIBRARY_NAME} ${MZ_ADD_LIBRARY_TYPE})
    mz_target_handle_warnings(${MZ_ADD_LIBRARY_NAME})
    target_compile_definitions(${MZ_ADD_LIBRARY_NAME} PUBLIC
        "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>"
        "$<$<CONFIG:Debug>:MZ_DEBUG>"
    )
    target_include_directories(${MZ_ADD_LIBRARY_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

function(mz_add_test_target)
    cmake_parse_arguments(
        MZ_ADD_TEST # prefix
        "" # options
        "" # single-value args
        "TARGET_NAME;TEST_COMMAND;PARENT_TARGET;SOURCES;DEPENDENCIES" # multi-value args
        ${ARGN})

    # Test targets are executable targets.
    qt_add_executable(${MZ_ADD_TEST_TARGET_NAME}
        ${MZ_ADD_TEST_SOURCES}
    )

    add_test(
        NAME ${MZ_ADD_TEST_TARGET_NAME}
        COMMAND ${MZ_ADD_TEST_TEST_COMMAND}
    )

    target_compile_definitions(${MZ_ADD_TEST_TARGET_NAME} PRIVATE
        UNIT_TEST
        "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>"
        "$<$<CONFIG:Debug>:MZ_DEBUG>"
    )

    target_link_libraries(${MZ_ADD_TEST_TARGET_NAME} PRIVATE Qt6::Test)
    target_link_libraries(${MZ_ADD_TEST_TARGET_NAME} PUBLIC
        ${MZ_ADD_TEST_PARENT_TARGET}
        ${MZ_ADD_TEST_DEPENDENCIES}
    )

    target_include_directories(${TEST_TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )
endfunction()

# Sets the following new variables in the parent scope:
#
# LINK_LIBRARIES
# TEST_LINK_LIBRARIES
function(mz_generate_link_libraries)
    cmake_parse_arguments(
        MZ_GENERATE_LINK_LIBRARIES # prefix
        "" # options
        "" # single-value args
        "QT_DEPENDENCIES;MZ_DEPENDENCIES;RUST_DEPENDENCIES;EXTRA_DEPENDENCIES;TEST_DEPENDENCIES;IOS_DEPENDENCIES;ANDROID_DEPENDENCIES;MACOS_DEPENDENCIES;LINUX_DEPENDENCIES;WINDOWS_DEPENDENCIES;WASM_DEPENDENCIES;DUMMY_DEPENDENCIES" # multi-value args
        ${ARGN})

    set(LOCAL_LINK_LIBRARIES)

    # 1. Qt dependencies handling

    # Get list of required Qt dependencies
    find_package(Qt6 REQUIRED COMPONENTS ${MZ_GENERATE_LINK_LIBRARIES_QT_DEPENDENCIES})
    foreach(QT_DEPENDENCY ${MZ_GENERATE_LINK_LIBRARIES_QT_DEPENDENCIES})
        list(APPEND LOCAL_LINK_LIBRARIES "Qt6::${QT_DEPENDENCY}")
    endforeach()

    # 2. MZ dependencies handling

    list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_MZ_DEPENDENCIES})

    # 3. Rust dependencies handling

    # Build Rust creates and add to list of linkd targets.
    foreach(RUST_CRATE_PATH ${MZ_GENERATE_LINK_LIBRARIES_RUST_DEPENDENCIES})
        # The name of the crate target is expected to be the name of the crate folder
        get_filename_component(CRATE_NAME ${RUST_CRATE_PATH} NAME)

        include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)
        add_rust_library(${CRATE_NAME}
            PACKAGE_DIR ${RUST_CRATE_PATH}
            BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
            CRATE_NAME ${CRATE_NAME}
        )

        list(APPEND LOCAL_LINK_LIBRARIES ${CRATE_NAME})
    endforeach()

    # 4. Platform specific dependencies handling

    if(${MZ_PLATFORM_NAME} STREQUAL "ios")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_IOS_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "android")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_ANDROID_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "macos")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_MACOS_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "linux")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_LINUX_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "windows")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_WINDOWS_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "wasm")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_WASM_DEPENDENCIES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "dummy")
        list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_DUMMY_DEPENDENCIES})
    else()
        message(FATAL_ERROR "MZ_PLATFORM_NAME must be set before creating modules.")
    endif()

    # 5. Finalize the list of all dependencies

    list(APPEND LOCAL_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_EXTRA_DEPENDENCIES})

    # 6. Test dependency handling

    set(LOCAL_TEST_LINK_LIBRARIES ${LOCAL_LINK_LIBRARIES})

    # Replace dependencies for test dependencies that start with `replace-`
    set(REPLACER_DEPENDENCIES ${MZ_GENERATE_LINK_LIBRARIES_TEST_DEPENDENCIES})
    list(FILTER REPLACER_DEPENDENCIES INCLUDE REGEX "^replace-")
    foreach(REPLACER_DEPENDENCY ${REPLACER_DEPENDENCIES})
        # Get the name of the original dependency
        string(REPLACE "replace-" "" ORIGINAL_DEPENDENCY ${REPLACER_DEPENDENCY})
        # Remove it  from the list
        list(REMOVE_ITEM LOCAL_TEST_LINK_LIBRARIES ${ORIGINAL_DEPENDENCY})
    endforeach()

    # Tests always need Qt6::Test
    find_package(Qt6 REQUIRED COMPONENTS Test)
    list(APPEND LOCAL_TEST_LINK_LIBRARIES Qt6::Test)

    # Finalize the list of test dependencies.
    list(APPEND LOCAL_TEST_LINK_LIBRARIES ${MZ_GENERATE_LINK_LIBRARIES_TEST_DEPENDENCIES})

    # 7. Set the lists in the parent scope.

    set(LINK_LIBRARIES ${LOCAL_LINK_LIBRARIES} PARENT_SCOPE)
    set(TEST_LINK_LIBRARIES ${LOCAL_TEST_LINK_LIBRARIES} PARENT_SCOPE)
endfunction()

# Sets the following new variables in the parent scope:
#
# ALL_SOURCES
# APPLE_SOURCES -- These are no included in ALL_SOURCES.
function(mz_generate_sources_list)
    cmake_parse_arguments(
        MZ_GENERATE_SOURCES_LIST # prefix
        "" # options
        "" # single-value args
        "SOURCES;IOS_SOURCES;ANDROID_SOURCES;MACOS_SOURCES;LINUX_SOURCES;WINDOWS_SOURCES;WASM_SOURCES;DUMMY_SOURCES" # multi-value args
        ${ARGN})

    set(LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_SOURCES})

    # 1. Handle platform specific sources

    if(${MZ_PLATFORM_NAME} STREQUAL "ios")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_IOS_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "android")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_ANDROID_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "macos")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_MACOS_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "linux")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_LINUX_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "windows")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_WINDOWS_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "wasm")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_WASM_SOURCES})
    elseif(${MZ_PLATFORM_NAME} STREQUAL "dummy")
        list(APPEND LOCAL_ALL_SOURCES ${MZ_GENERATE_SOURCES_LIST_DUMMY_SOURCES})
    else()
        message(FATAL_ERROR "MZ_PLATFORM_NAME must be set before creating modules.")
    endif()

    # 2. Separate out Swift and ObjC sources

    set(SWIFT_SOURCES ${LOCAL_ALL_SOURCES})
    list(FILTER SWIFT_SOURCES INCLUDE REGEX "\.swift$")

    set(OBJC_SOURCES ${LOCAL_ALL_SOURCES})
    list(FILTER OBJC_SOURCES INCLUDE REGEX "\.mm$")

    list(FILTER LOCAL_ALL_SOURCES EXCLUDE REGEX "\.swift$")
    list(FILTER LOCAL_ALL_SOURCES EXCLUDE REGEX "\.mm$")

    set(APPLE_SOURCES ${SWIFT_SOURCES} ${OBJC_SOURCES} PARENT_SCOPE)

    # 3. Set the list in the parent scope.

    set(ALL_SOURCES ${LOCAL_ALL_SOURCES} PARENT_SCOPE)
endfunction()

# Swift and Obj-C files are all added to a separate target that includes
# all apple stuff and the Obj-C bridging header.
#
# I (Bea) could not figure out how to make a static lib into an
# importable swift module, so this was easier. The ObjC stuff was added here as well,
# because otherwise ObjC files don't find the brdiging header. Hacky include directories
# shenanigans were considered, but we decided it was too hacky.
set(APPLE_SPECIFIC_TARGET_NAME mz_apple_stuff)
function(mz_add_the_apple_stuff)
    cmake_parse_arguments(
        MZ_ADD_APPLE_STUFF # prefix
        "" # options
        "" # single-value args
        "TARGET_NAME;SOURCES;DEPENDENCIES;INCLUDE_DIRECTORIES" # multi-value args
        ${ARGN})

    # Create the Swift target if it doesn't exist.
    if(NOT TARGET ${APPLE_SPECIFIC_TARGET_NAME})
        mz_add_library(
            NAME ${APPLE_SPECIFIC_TARGET_NAME}
            TYPE STATIC
        )
        set_target_properties(${APPLE_SPECIFIC_TARGET_NAME} PROPERTIES
            XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
            XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
            XCODE_ATTRIBUTE_SWIFT_OBJC_INTERFACE_HEADER_NAME "MozillaVPN-Swift.h"
            # Do not strip debug symbols on copy
            XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
            XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
            XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
        )
    endif()

    target_sources(${APPLE_SPECIFIC_TARGET_NAME} PUBLIC
        ${MZ_ADD_APPLE_STUFF_SOURCES}
    )
    target_include_directories(${APPLE_SPECIFIC_TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MZ_ADD_APPLE_STUFF_INCLUDE_DIRECTORIES}
    )
    target_link_libraries(${APPLE_SPECIFIC_TARGET_NAME} PRIVATE
        ${MZ_ADD_APPLE_STUFF_DEPENDENCIES}
    )

    target_link_libraries(${MZ_ADD_APPLE_STUFF_TARGET_NAME} PUBLIC
        ${APPLE_SPECIFIC_TARGET_NAME}
    )
endfunction()

