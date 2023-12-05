
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
