/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORITEMPICKER_H
#define INSPECTORITEMPICKER_H

#include "itempicker.h"

class InspectorItemPicker final : public ItemPicker {
 public:
  explicit InspectorItemPicker(QObject* parent);

 private:
  bool itemPicked(const QList<QQuickItem*>& list) override;
};

#endif  // INSPECTORITEMPICKER_H
