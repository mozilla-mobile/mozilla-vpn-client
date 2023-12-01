# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_link_options(-Wno-unused-command-line-argument)



target_sources(mozillavpn PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/dummy/dummycontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/dummy/dummycontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkwatcher.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkwatcher.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmwindowcontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmwindowcontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmiaphandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmiaphandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasminspector.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasminspector.h
     ${CMAKE_CURRENT_SOURCE_DIR}/systemtraynotificationhandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/systemtraynotificationhandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.h
)

# Install the Compile Artifacts
install(FILES ${CMAKE_BINARY_DIR}/src/mozillavpn.js DESTINATION .)
install(FILES ${CMAKE_BINARY_DIR}/src/mozillavpn.wasm DESTINATION .)
install(FILES ${CMAKE_BINARY_DIR}/src/qtloader.js DESTINATION .)
# Install the Emulator for Guardian and FXA
install(FILES ${CMAKE_SOURCE_DIR}/tests/functional/servers/fxa_endpoints.js DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/tests/functional/servers/guardian_endpoints.js DESTINATION .)

# Install the static runner web-page
set(WASM_RUNTIME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../wasm)
file(GLOB_RECURSE WASM_FILES ${WASM_RUNTIME_SOURCE_DIR}/*)
foreach(WASM_FILE ${WASM_FILES})
    get_filename_component(FILE_NAME ${WASM_FILE} NAME)
    install(FILES ${WASM_FILE} DESTINATION .)
endforeach()

add_compile_definitions("MZ_DUMMY")
