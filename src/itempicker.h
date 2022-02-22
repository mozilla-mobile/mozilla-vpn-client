/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ITEMPICKER_H
#define ITEMPICKER_H

#include <QObject>

class QMouseEvent;
class QQuickItem;

class ItemPicker : public QObject {
  Q_OBJECT

 public:
  explicit ItemPicker(QObject* parent);

 protected:
  // Return true if the event should be accepted and not propagated.
  virtual bool itemPicked(const QStringList& list) = 0;

 private:
  bool eventFilter(QObject* obj, QEvent* event) override;

  QStringList pickItem(QMouseEvent* event, QQuickItem* item);

 private:
  QEvent* m_lastEvent = nullptr;
};

#endif  // ITEMPICKER_H
