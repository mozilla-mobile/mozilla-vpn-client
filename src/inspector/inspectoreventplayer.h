/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOREVENTPLAYER_H
#define INSPECTOREVENTPLAYER_H

#include <QEvent>
#include <QObject>
#include <QTimer>

class InspectorEventPlayer final : public QObject {
  Q_OBJECT

 public:
  InspectorEventPlayer(QObject* parent, const QString& fileName);

 private:
  void processNextEvent();
  void dispatchEvent();

 private:
  struct Event {
    QEvent* m_event = nullptr;
    qint64 m_time;
  };

  QList<Event> m_events;
  QTimer m_timer;
};

#endif  // INSPECTOREVENTPLAYER_H
