# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.



# This file adds a function 
# mz_add_clang_tidy( <SOME_TARGET> )
#
# <Some_TARGET> -> A target containing c++ source files.
# 
# It will generate 2 targets
#
# <some_target>_clang_tidy -> Runs Clang Tidy for all files in that target
# <some_target>_clang_tidy_fix -> Runs Clang Tidy for all files in that target and attempts to fix them 
#
# Also it will add 2 targets:
# "clang_tidy_all" "clang_tidy_fix_all", to run all 
#  respective targets at once for convinence :) 



## First Check that we can run clang tidy: 
# We need: 
# -> Clang tidy to be found 
# -> the request for compile_commands to be generated 
# -> a generator that can actually generate that file
# 
# If they are not fullfilled, generate a dummy function and exit.
if(ANDROID)
    # On Android use the NDK provided one
    find_program(CLANG_TIDY_EXECUTABLE 
        NAMES clang-tidy
        NO_DEFAULT_PATH
        HINTS ${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin/
        # TODO: add path hints for macos and windows too. 
    )
else()
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
endif()


# Ensure clang-tidy is found
if(NOT CLANG_TIDY_EXECUTABLE)
    message(STATUS "clang-tidy: Binary not found. Linting will be skipped.")
    function(mz_add_clang_tidy dummyArgument)
    endfunction() 
    return()
endif()
# Check if the flag is set
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    message(STATUS "clang-tidy: The -DEXPORT_COMPILE_COMMANDS=ON flag is not set. Linting will be skipped.")
    function(mz_add_clang_tidy dummyArgument)
    endfunction() 
    return()
endif()
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    message(STATUS "clang-tidy: Clang is not used as compiler. Linting will be skipped.")
    function(mz_add_clang_tidy dummyArgument)
    endfunction() 
    return()
endif()


add_custom_target(clang_tidy_report)

# Create a folder for clang-tidy reports
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/clang-tidy)
function(mz_add_clang_tidy aTarget)

    get_target_property(aTarget_SOURCE_FILES ${aTarget} SOURCES)
    get_target_property(aTarget_INTERFACE_FILES ${aTarget} INTERFACE_SOURCES)
    list(APPEND aTarget_SOURCE_FILES ${aTarget_INTERFACE_FILES})

    # Filter for .cpp files and check source directory location 
    # Otherwise we might have files from MOC there. 
    list(FILTER aTarget_SOURCE_FILES INCLUDE REGEX ".*\\.cpp$")
    list(FILTER aTarget_SOURCE_FILES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")
   
    if(WIN32)
        # on windows we need to pass \\ to clang tidy
        # cmake by default uses / as delemiter so let's replace that. 
        foreach(file ${aTarget_SOURCE_FILES})
            string(REPLACE "/" "\\\\" fixed_path ${file})
            list(APPEND temp_fixed_paths ${fixed_path})
        endforeach()
        set(aTarget_SOURCE_FILES ${temp_fixed_paths})
    endif()

    add_custom_target(${aTarget}_clang_tidy_report
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/run-clang-tidy.py -clang-tidy-binary ${CLANG_TIDY_EXECUTABLE} -export-fixes=${CMAKE_BINARY_DIR}/clang-tidy/${aTarget}-report.yaml -p ${CMAKE_BINARY_DIR} ${aTarget_SOURCE_FILES}
        DEPENDS ${aTarget}
        COMMENT "Build clang-tidy report for ${aTarget}"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} # Needs to be root so the config file is found
    )
    # Add the clang-tidy targets as a dependency 
    add_dependencies(clang_tidy_report ${aTarget}_clang_tidy_report)
endfunction() 
