# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(mozillavpn PRIVATE
    socks5proxyhandler.cpp
    socks5proxyhandler.h
)

add_definitions(-DMVPN_SOCKS5PROXY)
