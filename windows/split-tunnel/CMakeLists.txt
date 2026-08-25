# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(FetchContent)

FetchContent_Declare(
    win_split_tunnel
    DOWNLOAD_EXTRACT_TIMESTAMP true
    GIT_REPOSITORY https://github.com/mullvad/mullvadvpn-app-binaries.git
    GIT_TAG cc0affb2f06e870fb594e2dd6d61049611991586 # Latest commit as of August 24, 2026
    # We only need the prebuilt driver files, which live in the top-level tree.
    # Skip the submodules: libmnl and libnftnl are Linux-only netfilter libraries
    # served over git:// from git.netfilter.org, which CI cannot reach.
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(win_split_tunnel)

if(CMAKE_C_COMPILER_TARGET)
    set(MULLVAD_TARGET_SOURCE_DIR ${win_split_tunnel_SOURCE_DIR}/${CMAKE_C_COMPILER_TARGET})
else()
    set(MULLVAD_TARGET_SOURCE_DIR ${win_split_tunnel_SOURCE_DIR}/x86_64-pc-windows-msvc)
endif()

# Install the prebuilt binaries
install(FILES
    ${MULLVAD_TARGET_SOURCE_DIR}/split-tunnel/mullvad-split-tunnel.cat
    ${MULLVAD_TARGET_SOURCE_DIR}/split-tunnel/mullvad-split-tunnel.inf
    ${MULLVAD_TARGET_SOURCE_DIR}/split-tunnel/mullvad-split-tunnel.sys
    DESTINATION .
)
