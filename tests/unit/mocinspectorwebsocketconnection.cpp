/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/inspector/inspectorwebsocketconnection.h"

InspectorWebSocketConnection::InspectorWebSocketConnection(
    QObject* parent, QWebSocket* connection) {}

InspectorWebSocketConnection::~InspectorWebSocketConnection() = default;

void InspectorWebSocketConnection::setLastUrl(const QUrl& url) {}

bool InspectorWebSocketConnection::stealUrls() { return false; }
