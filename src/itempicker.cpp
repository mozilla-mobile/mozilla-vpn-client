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
    return QObject::eventFilter(obj, event);
  }

  m_lastEvent = event;

  if (event->type() != QEvent::MouseButtonPress) {
    return QObject::eventFilter(obj, event);
  }

  QQuickItem* item = qobject_cast<QQuickItem*>(obj);
  if (!item) {
    QQuickWindow* window = qobject_cast<QQuickWindow*>(obj);
    if (window) {
      item = window->contentItem();
    }
  }

  if (!item) {
    return QObject::eventFilter(obj, event);
  }

  QStringList list = pickItem(static_cast<QMouseEvent*>(event), item);
  if (itemPicked(list)) {
    event->setAccepted(true);
    return true;
  }

  return QObject::eventFilter(obj, event);
}

QStringList ItemPicker::pickItem(QMouseEvent* event, QQuickItem* item) {
  QStringList list;
  if (!item->objectName().isEmpty()) {
    list.append(item->objectName());
  }

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

  QQuickItem* contentItem = item->property("contentItem").value<QQuickItem*>();
  if (contentItem) {
    for (QQuickItem* child : contentItem->childItems()) {
      if (!child->isVisible() || !child->isEnabled()) {
        continue;
      }

      QPointF gpos = child->mapFromGlobal(event->globalPos());
      if (!child->contains(gpos)) {
        continue;
      }

      if (!child->contains(gpos)) {
        continue;
      }

      list.append(pickItem(event, child));
    }
  }

  return list;
}
