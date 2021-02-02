/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <QObject>
#include <QLocalServer>

class EventListener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(EventListener)

 public:
  EventListener();
  ~EventListener();

  static bool checkOtherInstances();

 private:
  QLocalServer m_server;
};

#endif  // EVENTLISTENER_H
