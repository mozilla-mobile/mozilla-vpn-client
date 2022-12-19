# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(mozillavpn PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/dummy/dummycontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/dummy/dummycontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/macos/macosmenubar.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/macos/macosmenubar.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmauthenticationlistener.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmauthenticationlistener.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmnetworkrequest.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmnetworkrequest.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmnetworkwatcher.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmnetworkwatcher.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmwindowcontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmwindowcontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmiaphandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasmiaphandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasminspector.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/wasm/wasminspector.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/systemtraynotificationhandler.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/systemtraynotificationhandler.h
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.h
)

add_compile_definitions("MZ_DUMMY")
