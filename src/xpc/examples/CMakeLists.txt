



if (TARGET build_examples)
    # -- 
else()
    add_custom_target(build_examples)
    set_target_properties(build_examples PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        FOLDER "examples"
    )
endif()

qt_add_executable(xpc_test_client client.cpp)
set_target_properties(xpc_test_client PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(build_examples xpc_test_client)
target_sources(xpc_test_client PUBLIC 
    ../../leakdetector.h
    ../../leakdetector.cpp
    ../xpcbase.h
    ../xpcclient.h 
    ../xpcclient.cpp
)
target_link_libraries(xpc_test_client PRIVATE
"-framework CoreFoundation"
"-framework Foundation"
"-framework IOKit"
)



qt_add_executable(xpc_test_server server.cpp )
target_sources(xpc_test_server PUBLIC 
    ../../leakdetector.h
    ../../leakdetector.cpp
    ../xpcbase.h
    ../xpcservice.h 
    ../xpcservice.cpp
)
target_include_directories(xpc_test_server PUBLIC 
    ../ 
    ../../
)

target_link_libraries(xpc_test_server PRIVATE
"-framework CoreFoundation"
"-framework Foundation"
"-framework IOKit"
)

set_target_properties(xpc_test_server PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(build_examples xpc_test_server)


set(SERVER_OUTPUT $<TARGET_FILE:xpc_test_server>)
configure_file(example.server.plist.in example.server.plist @ONLY)
