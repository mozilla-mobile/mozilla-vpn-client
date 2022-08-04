# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(mozillavpn PRIVATE
    platforms/dummy/dummycontroller.cpp
    platforms/dummy/dummycontroller.h
    platforms/dummy/dummycryptosettings.cpp
    platforms/macos/macosmenubar.cpp
    platforms/macos/macosmenubar.h
    platforms/wasm/wasmauthenticationlistener.cpp
    platforms/wasm/wasmauthenticationlistener.h
    platforms/wasm/wasmnetworkrequest.cpp
    platforms/wasm/wasmnetworkrequest.h
    platforms/wasm/wasmnetworkwatcher.cpp
    platforms/wasm/wasmnetworkwatcher.h
    platforms/wasm/wasmwindowcontroller.cpp
    platforms/wasm/wasmwindowcontroller.h
    platforms/wasm/wasmiaphandler.cpp
    platforms/wasm/wasmiaphandler.h
    platforms/wasm/wasminspector.cpp
    platforms/wasm/wasminspector.h
    systemtraynotificationhandler.cpp
    systemtraynotificationhandler.h
    tasks/purchase/taskpurchase.cpp
    tasks/purchase/taskpurchase.h
)

add_compile_definitions("MVPN_DUMMY")
