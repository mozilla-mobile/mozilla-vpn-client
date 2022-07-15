# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network

TARGET = proxy
SOURCES = bin/main.cpp src/socks5.cpp src/socks5client.cpp
HEADERS = src/socks5.h src/socks5client.h

INCLUDEPATH += src
