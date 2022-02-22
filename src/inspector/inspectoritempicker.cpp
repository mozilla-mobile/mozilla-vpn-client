/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectoritempicker.h"
#include "inspectorwebsocketconnection.h"

InspectorItemPicker::InspectorItemPicker(QObject* parent)
    : ItemPicker(parent) {}

bool InspectorItemPicker::itemPicked(const QStringList& list) {
  InspectorWebSocketConnection::itemsPicked(list);
  return true;
}
