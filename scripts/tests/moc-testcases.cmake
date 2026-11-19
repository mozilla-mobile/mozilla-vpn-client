# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(moc_generate_testcases TARGET)
    get_target_property(TARGET_BINARY_DIR ${TARGET} BINARY_DIR)
    get_target_property(TARGET_SOURCE_DIR ${TARGET} SOURCE_DIR)

    cmake_parse_arguments(MOC
        ""
        "PARENT_CLASS"
        ""
        ${ARGN})

    ## A parent class must be specified from which testcases are derived
    if(NOT MOC_PARENT_CLASS)
        message(FATAL_ERROR "moc_generate_testcases missing required PARENT_CLASS argument")
    endif()

    # Auto-generate the testcase list by post-processing the moc output.
    qt_extract_metatypes(${TARGET} OUTPUT_FILES TARGET_METATYPE_JSON)
    add_custom_command(
        OUTPUT ${TARGET_BINARY_DIR}/${TARGET}-testcases.cpp
        DEPENDS
            ${TARGET_METATYPE_JSON} 
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/moc-testcases.py
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/moc-testcases.py
            --parent ${MOC_PARENT_CLASS} --output ${TARGET_BINARY_DIR}/${TARGET}-testcases.cpp
             ${TARGET_METATYPE_JSON} 
    )
    target_sources(${TARGET} PRIVATE ${TARGET_BINARY_DIR}/${TARGET}-testcases.cpp)
    set_source_files_properties(${TARGET_BINARY_DIR}/${TARGET}-testcases.cpp PROPERTIES
        INCLUDE_DIRECTORIES ${TARGET_SOURCE_DIR}
        GENERATED TRUE
        SKIP_AUTOGEN TRUE
    )
endfunction()
