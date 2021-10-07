/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectoreventrecorder.h"
#include "inspectorwebsocketconnection.h"

#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickItem>
#include <QQuickWindow>

InspectorEventRecorder::InspectorEventRecorder(
    QObject* parent, InspectorEventRecorder::Mode mode, const QString& fileName,
    bool allEvents)
    : QObject(parent),
      m_mode(mode),
      m_fileName(fileName),
      m_allEvents(allEvents) {}

bool InspectorEventRecorder::eventFilter(QObject* obj, QEvent* event) {
  if (m_lastEvent == event) {
    return QObject::eventFilter(obj, event);
  }

  m_lastEvent = event;

  switch (m_mode) {
    case Picker:
      return eventFilterPicker(obj, event);

    case Recorder:
      return eventFilterRecorder(obj, event);

    default:
      Q_ASSERT(false);
      return QObject::eventFilter(obj, event);
  }
}

bool InspectorEventRecorder::eventFilterPicker(QObject* obj, QEvent* event) {
  Q_ASSERT(m_mode == Picker);

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

  QStringList list = pickElement(static_cast<QMouseEvent*>(event), item);
  InspectorWebSocketConnection::pickedElements(list);

  event->setAccepted(true);
  return true;
}

bool InspectorEventRecorder::eventFilterRecorder(QObject* obj, QEvent* event) {
  Q_ASSERT(m_mode == Recorder);

  switch (event->type()) {
    case QEvent::MouseButtonPress:
      [[fallthrough]];
    case QEvent::MouseButtonRelease:
      trackMouseEvent(static_cast<QMouseEvent*>(event));
      break;

    case QEvent::MouseMove:
      if (m_allEvents) {
        trackMouseEvent(static_cast<QMouseEvent*>(event));
      }
      break;

    case QEvent::HoverEnter:
      [[fallthrough]];
    case QEvent::HoverLeave:
      [[fallthrough]];
    case QEvent::HoverMove:
      if (m_allEvents) {
        trackHoverEvent(static_cast<QHoverEvent*>(event));
      }
      break;

    case QEvent::KeyPress:
      [[fallthrough]];
    case QEvent::KeyRelease:
      trackKeyEvent(static_cast<QKeyEvent*>(event));
      break;

    case QEvent::Wheel:
      if (m_allEvents) {
        trackWheelEvent(static_cast<QWheelEvent*>(event));
      }
      break;

    default:
      break;
  }

  return QObject::eventFilter(obj, event);
}

void InspectorEventRecorder::trackMouseEvent(QMouseEvent* event) {
  QJsonObject obj;
  obj["type"] = event->type();
  obj["x"] = event->localPos().x();
  obj["y"] = event->localPos().y();
  obj["button"] = static_cast<int>(event->button());
  obj["buttons"] = static_cast<int>(event->buttons());
  obj["modifiers"] = static_cast<int>(event->modifiers());
  obj["time"] = m_timer.isValid() ? m_timer.elapsed() : 0;
  m_timer.start();

  trackEvent(obj);
}

void InspectorEventRecorder::trackHoverEvent(QHoverEvent* event) {
  QJsonObject obj;
  obj["type"] = event->type();
  obj["x"] = event->posF().x();
  obj["y"] = event->posF().y();
  obj["ox"] = event->oldPosF().x();
  obj["oy"] = event->oldPosF().y();
  obj["modifiers"] = static_cast<int>(event->modifiers());
  obj["time"] = m_timer.isValid() ? m_timer.elapsed() : 0;
  m_timer.start();

  trackEvent(obj);
}

void InspectorEventRecorder::trackKeyEvent(QKeyEvent* event) {
  QJsonObject obj;
  obj["type"] = event->type();
  obj["key"] = event->key();
  obj["text"] = event->text();
  obj["modifiers"] = static_cast<int>(event->modifiers());
  obj["autorep"] = event->isAutoRepeat();
  obj["count"] = event->count();
  obj["time"] = m_timer.isValid() ? m_timer.elapsed() : 0;
  m_timer.start();

  trackEvent(obj);
}

void InspectorEventRecorder::trackWheelEvent(QWheelEvent* event) {
  QJsonObject obj;
  obj["type"] = event->type();
  obj["x"] = event->position().x();
  obj["y"] = event->position().y();
  obj["gpx"] = event->globalPosition().x();
  obj["gpy"] = event->globalPosition().y();
  obj["pixeldeltax"] = event->pixelDelta().x();
  obj["pixeldeltay"] = event->pixelDelta().y();
  obj["angledeltax"] = event->angleDelta().x();
  obj["angledeltay"] = event->angleDelta().y();
  obj["buttons"] = static_cast<int>(event->buttons());
  obj["modifiers"] = static_cast<int>(event->modifiers());
  obj["phase"] = static_cast<int>(event->phase());
  obj["inverted"] = event->inverted();
  obj["source"] = static_cast<int>(event->source());
  obj["time"] = m_timer.isValid() ? m_timer.elapsed() : 0;
  m_timer.start();

  trackEvent(obj);
}

void InspectorEventRecorder::trackEvent(const QJsonObject& obj) {
  if (!m_eventFile) {
    m_eventFile = new QFile(m_fileName, this);
    if (!m_eventFile->open(QIODevice::WriteOnly)) {
      delete m_eventFile;
      m_eventFile = nullptr;
      return;
    }
  }

  QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
  json.append("\n");

  m_eventFile->write(json, json.length());
}

QStringList InspectorEventRecorder::pickElement(QMouseEvent* event,
                                                QQuickItem* item) {
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

    list.append(pickElement(event, child));
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

      list.append(pickElement(event, child));
    }
  }

  return list;
}
