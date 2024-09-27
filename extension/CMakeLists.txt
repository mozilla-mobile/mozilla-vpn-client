# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Custom target to build the web-extension bridge with Rust/Cargo

if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    add_custom_target(cargo_mozillavpnnp_all COMMENT "Build vpnnp for all Architectures")
    # For each requested Arch , create a new target and 
    # make it a dependency for cargo_mozillavpnnp_all
    foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
        string(REPLACE "arm64" "aarch64" OSXARCH ${OSXARCH})
        add_custom_target(cargo_mozillavpnnp_${OSXARCH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bridge
            COMMAND ${CMAKE_COMMAND} -E env CARGO_HOME=${CMAKE_CURRENT_BINARY_DIR}/.cargo_home 
                cargo build --release --target-dir "${CMAKE_CURRENT_BINARY_DIR}/bridge"
                --target "${OSXARCH}-apple-darwin"
        )
        list(APPEND MOZILLAVPNNP_TARGET_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/bridge/${OSXARCH}-apple-darwin/release/mozillavpnnp")
        add_dependencies(cargo_mozillavpnnp_all cargo_mozillavpnnp_${OSXARCH})
    endforeach()
    # Create a target that runs after all-arches have been build 
    # Create a far binary using all lipo
    add_custom_target(cargo_mozillavpnnp ALL
        COMMENT "Create a universal binary for mozillavpnp"
        DEPENDS cargo_mozillavpnnp_all
        COMMAND lipo -create -output ${CMAKE_CURRENT_BINARY_DIR}/bridge/target/release/mozillavpnnp ${MOZILLAVPNNP_TARGET_OUTPUTS}
    )
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bridge/target/release/)
else()
    # Single Plattform 
    add_custom_target(cargo_mozillavpnnp ALL
        COMMENT "Building web extension crate"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bridge
        COMMAND ${CMAKE_COMMAND} -E env CARGO_HOME=${CMAKE_CURRENT_BINARY_DIR}/.cargo_home
                cargo build --release --target-dir ${CMAKE_CURRENT_BINARY_DIR}/bridge/target
    )
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/bridge/target)
endif()

## Wrap the build artifacts as a CMake target.
add_executable(mozillavpnnp IMPORTED GLOBAL)
add_dependencies(mozillavpnnp cargo_mozillavpnnp)
set_property(TARGET mozillavpnnp PROPERTY
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/bridge/target/release/mozillavpnnp${CMAKE_EXECUTABLE_SUFFIX})

## Handle installation by platform.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if(NOT DEFINED WEBEXT_INSTALL_LIBDIR)
        set(WEBEXT_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
    endif()

    install(PROGRAMS $<TARGET_FILE:mozillavpnnp> DESTINATION ${WEBEXT_INSTALL_LIBDIR}/mozillavpn)

    configure_file(manifests/linux/mozillavpn.json.in manifests/linux/mozillavpn.json)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION ${WEBEXT_INSTALL_LIBDIR}/mozilla/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION /etc/opt/chrome/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION /etc/chromium/native-messaging-hosts)
elseif(WIN32)
    install(PROGRAMS $<TARGET_FILE:mozillavpnnp> DESTINATION .)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/manifests/windows/mozillavpn.json
        DESTINATION .)
endif()

add_subdirectory(socks5proxy)
