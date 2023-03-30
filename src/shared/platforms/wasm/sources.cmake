# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(MZ_SHARED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/shared ABSOLUTE)

target_sources(shared-sources INTERFACE
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmauthenticationlistener.cpp
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmauthenticationlistener.h
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmcryptosettings.cpp
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmfontloader.cpp
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmnetworkrequest.cpp
     ${MZ_SHARED_SOURCE_DIR}/platforms/wasm/wasmnetworkrequest.h
)
