/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSEVENTLISTENER_H
#define WINDOWSEVENTLISTENER_H

#include <QObject>
#include <QLocalServer>

class WindowsEventListener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsEventListener)

 public:
  WindowsEventListener();
  ~WindowsEventListener();

  static bool checkOtherInstances();

 private:
  QLocalServer m_server;
};

#endif  // WINDOWSEVENTLISTENER_H
