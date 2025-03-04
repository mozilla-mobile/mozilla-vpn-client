# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

list(APPEND QT_WASM_EXTRA_EXPORTED_METHODS ENV)

target_sources(mozillavpn PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmcontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmcontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkwatcher.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkwatcher.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmwindowcontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmwindowcontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmiaphandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmiaphandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/systemtraynotificationhandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/systemtraynotificationhandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/tasks/purchase/taskpurchase.h
)
set_target_properties(mozillavpn PROPERTIES
  LINK_FLAGS "-gseparate-dwarf=${CMAKE_BINARY_DIR}/src/mozillavpn.debug.wasm")


set(WASM_FINAL_DIR ${CMAKE_BINARY_DIR}/wasm_build)
add_custom_command(
     TARGET mozillavpn
     POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/wasm ${WASM_FINAL_DIR}
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/mozillavpn.js ${WASM_FINAL_DIR}/mozillavpn.js
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/mozillavpn.wasm ${WASM_FINAL_DIR}/mozillavpn.wasm
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/mozillavpn.debug.wasm ${WASM_FINAL_DIR}/mozillavpn.debug.wasm
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/qtloader.js ${WASM_FINAL_DIR}/qtloader.js
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/functional/servers/fxa_endpoints.js ${WASM_FINAL_DIR}/fxa_endpoints.js
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/functional/servers/guardian_endpoints.js ${WASM_FINAL_DIR}/guardian_endpoints.js
     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tools/logviewer ${WASM_FINAL_DIR}/logviewer
)
