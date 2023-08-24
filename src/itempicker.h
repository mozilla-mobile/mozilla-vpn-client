/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ITEMPICKER_H
#define ITEMPICKER_H

#include <QObject>

class QMouseEvent;
class QQuickItem;
class QTouchEvent;

class ItemPicker : public QObject {
  Q_OBJECT

 public:
  explicit ItemPicker(QObject* parent);

 protected:
  // Return true if the event should be accepted and not propagated.
  virtual bool itemPicked(const QList<QQuickItem*>& list) = 0;

 private:
  bool eventFilter(QObject* obj, QEvent* event) override;
  bool eventFilterInternal(QObject* obj, QEvent* event);

  QList<QQuickItem*> pickItem(QMouseEvent* event, QQuickItem* item);
  QList<QQuickItem*> pickItem(QTouchEvent* event, QQuickItem* item);

 private:
  QEvent* m_lastEvent = nullptr;
  bool m_lastResponse = false;
};

#endif  // ITEMPICKER_H
