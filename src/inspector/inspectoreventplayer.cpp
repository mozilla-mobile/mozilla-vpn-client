/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectoreventplayer.h"
#include "inspectorwebsocketconnection.h"
#include "qmlengineholder.h"

#include <QElapsedTimer>
#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QMouseEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWindow>

InspectorEventPlayer::InspectorEventPlayer(QObject* parent,
                                           const QString& fileName)
    : QObject(parent) {
  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this,
          &InspectorEventPlayer::dispatchEvent);

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    InspectorWebSocketConnection::eventReplayCompleted();
    return;
  }

  QByteArray content = file.readAll();
  QList<QByteArray> entries = content.split('\n');
  for (const QByteArray& entry : entries) {
    QJsonDocument doc = QJsonDocument::fromJson(entry);
    if (!doc.isObject()) {
      continue;
    }

    QJsonObject obj = doc.object();

    QEvent::Type type = static_cast<QEvent::Type>(obj["type"].toInt());
    if (type == QEvent::MouseButtonPress ||
        type == QEvent::MouseButtonRelease || type == QEvent::MouseMove) {
      QPointF pos(obj["x"].toDouble(), obj["y"].toDouble());
      Qt::MouseButton mouseButton =
          static_cast<Qt::MouseButton>(obj["button"].toInt());
      Qt::MouseButtons mouseButtons =
          static_cast<Qt::MouseButtons>(obj["buttons"].toInt());
      Qt::KeyboardModifiers modifiers =
          static_cast<Qt::KeyboardModifiers>(obj["modifiers"].toInt());

      m_events.append(Event{
          new QMouseEvent(type, pos, mouseButton, mouseButtons, modifiers),
          obj["time"].toInt()});
      continue;
    }

    if (type == QEvent::HoverEnter || type == QEvent::HoverLeave ||
        type == QEvent::HoverMove) {
      QPointF pos(obj["x"].toDouble(), obj["y"].toDouble());
      QPointF oldPos(obj["ox"].toDouble(), obj["oy"].toDouble());
      Qt::KeyboardModifiers modifiers =
          static_cast<Qt::KeyboardModifiers>(obj["modifiers"].toInt());
      m_events.append(Event{new QHoverEvent(type, pos, oldPos, modifiers),
                            obj["time"].toInt()});
      continue;
    }

    if (type == QEvent::KeyPress || type == QEvent::KeyRelease) {
      int key = obj["key"].toInt();
      Qt::KeyboardModifiers modifiers =
          static_cast<Qt::KeyboardModifiers>(obj["modifiers"].toInt());
      QString text = obj["text"].toString();
      bool autorep = obj["autorep"].toBool();
      ushort count = (ushort)obj["count"].toInt();
      m_events.append(
          Event{new QKeyEvent(type, key, modifiers, text, autorep, count),
                obj["time"].toInt()});
      continue;
    }

    if (type == QEvent::Wheel) {
      QPointF pos(obj["x"].toDouble(), obj["y"].toDouble());
      QPointF globalPos(obj["gpx"].toDouble(), obj["gpy"].toDouble());
      QPoint pixelDelta(obj["pixeldeltax"].toInt(), obj["pixeldeltay"].toInt());
      QPoint angleDelta(obj["angledeltax"].toInt(), obj["angledeltay"].toInt());
      Qt::MouseButtons buttons =
          static_cast<Qt::MouseButtons>(obj["buttons"].toInt());
      Qt::KeyboardModifiers modifiers =
          static_cast<Qt::KeyboardModifiers>(obj["modifiers"].toInt());
      Qt::ScrollPhase phase =
          static_cast<Qt::ScrollPhase>(obj["phase"].toInt());
      bool inverted = obj["inverted"].toBool();
      Qt::MouseEventSource source =
          static_cast<Qt::MouseEventSource>(obj["source"].toInt());
      m_events.append(
          Event{new QWheelEvent(pos, globalPos, pixelDelta, angleDelta, buttons,
                                modifiers, phase, inverted, source),
                obj["time"].toInt()});
      continue;
    }
  }

  processNextEvent();
}

void InspectorEventPlayer::processNextEvent() {
  if (m_events.isEmpty()) {
    InspectorWebSocketConnection::eventReplayCompleted();
    return;
  }

  m_timer.start(m_events[0].m_time);
}

void InspectorEventPlayer::dispatchEvent() {
  Q_ASSERT(!m_events.isEmpty());

  qApp->sendEvent(QmlEngineHolder::instance()->window(), m_events[0].m_event);

  delete m_events[0].m_event;
  m_events.removeAt(0);
  processNextEvent();
}
