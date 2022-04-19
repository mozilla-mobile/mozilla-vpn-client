function(add_go_library TARGET NAME)
    get_filename_component(ABS_NAME ${NAME} ABSOLUTE)
    get_filename_component(DIR_NAME ${ABS_NAME} DIRECTORY)
    get_filename_component(SRC_NAME ${NAME} NAME)
    string(REGEX REPLACE "[.]go$" ".a" ARCHIVE_NAME ${ABS_NAME})
    string(REGEX REPLACE "[.]go$" ".h" HEADER_NAME ${ABS_NAME})
    file(GLOB_RECURSE SRC_DEPS ${DIR_NAME}/*.go)

    target_sources(${TARGET} PRIVATE ${HEADER_NAME})
    set_source_files_properties(${HEADER_NAME} PROPERTIES GENERATED 1)

    add_custom_command(
        OUTPUT ${ARCHIVE_NAME} ${HEADER_NAME}
        WORKING_DIRECTORY ${DIR_NAME}
        MAIN_DEPENDENCY ${NAME}
        DEPENDS ${SRC_DEPS}
        COMMAND go build -buildmode=c-archive ${GOFLAGS} -v -o ${ARCHIVE_NAME} ${SRC_NAME}
    )
    target_link_libraries(${TARGET} PRIVATE ${ARCHIVE_NAME})
endfunction(add_go_library)
