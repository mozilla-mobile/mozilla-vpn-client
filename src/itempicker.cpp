/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "itempicker.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QQuickItem>
#include <QQuickWindow>

ItemPicker::ItemPicker(QObject* parent) : QObject(parent) {}

bool ItemPicker::eventFilter(QObject* obj, QEvent* event) {
  if (m_lastEvent == event) {
    return m_lastResponse;
  }

  m_lastEvent = event;
  m_lastResponse = eventFilterInternal(obj, event);

  return m_lastResponse;
}

bool ItemPicker::eventFilterInternal(QObject* obj, QEvent* event) {
  bool isMouseEvent = event->type() == QEvent::MouseButtonPress ||
                      event->type() == QEvent::MouseButtonDblClick ||
                      event->type() == QEvent::MouseButtonRelease;
  bool isTouchEvent =
      (event->type() == QEvent::TouchEnd ||
       event->type() == QEvent::TouchBegin) &&
      !!qobject_cast<QQuickWindow*>(static_cast<QTouchEvent*>(event)->target());

  if (
#if defined(MZ_ANDROID) || defined(MZ_IOS)
      !isTouchEvent
#else
      !isMouseEvent
#endif
  ) {
    return QObject::eventFilter(obj, event);
  }

  QQuickItem* item = qobject_cast<QQuickItem*>(obj);
  if (!item) {
    auto* window = qobject_cast<QQuickWindow*>(obj);
    if (window) {
      item = window->contentItem();
    }
  }

  if (!item) {
    return QObject::eventFilter(obj, event);
  }

  QList<QQuickItem*> list;

  if (isMouseEvent) {
    list = pickItem(static_cast<QMouseEvent*>(event), item);
  }

  if (isTouchEvent) {
    list = pickItem(static_cast<QTouchEvent*>(event), item);
  }

  if (itemPicked(list)) {
    if (isTouchEvent) {
      static_cast<QTouchEvent*>(event)->ignore();
    }
    event->setAccepted(true);
    return true;
  }

  return QObject::eventFilter(obj, event);
}

QList<QQuickItem*> ItemPicker::pickItem(QMouseEvent* event, QQuickItem* item) {
  QList<QQuickItem*> list;
  list.append(item);

  for (QQuickItem* child : item->childItems()) {
    if (!child->isVisible() || !child->isEnabled()) {
      continue;
    }

    QPointF gpos = child->mapFromGlobal(event->globalPos());
    if (!child->contains(gpos)) {
      continue;
    }

    list.append(pickItem(event, child));
  }

  auto* contentItem = item->property("contentItem").value<QQuickItem*>();
  if (contentItem) {
    for (QQuickItem* child : contentItem->childItems()) {
      if (!child->isVisible() || !child->isEnabled()) {
        continue;
      }

      QPointF gpos = child->mapFromGlobal(event->globalPos());
      if (!child->contains(gpos)) {
        continue;
      }

      list.append(pickItem(event, child));
    }
  }

  return list;
}

QList<QQuickItem*> ItemPicker::pickItem(QTouchEvent* event, QQuickItem* item) {
  QList<QQuickItem*> list;

  if (event->touchPoints().length() != 1) {
    return list;
  }

  list.append(item);

  QTouchEvent::TouchPoint point = event->touchPoints()[0];
  QPointF pos = point.pos();

  for (QQuickItem* child : item->childItems()) {
    if (!child->isVisible() || !child->isEnabled()) {
      continue;
    }

    QPointF gpos = child->mapFromGlobal(pos);
    if (!child->contains(gpos)) {
      continue;
    }

    list.append(pickItem(event, child));
  }

  auto* contentItem = item->property("contentItem").value<QQuickItem*>();
  if (contentItem) {
    for (QQuickItem* child : contentItem->childItems()) {
      if (!child->isVisible() || !child->isEnabled()) {
        continue;
      }

      QPointF gpos = child->mapFromGlobal(pos);
      if (!child->contains(gpos)) {
        continue;
      }

      list.append(pickItem(event, child));
    }
  }

  return list;
}
