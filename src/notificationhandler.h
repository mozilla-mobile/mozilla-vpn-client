/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include <QObject>

class NotificationHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NotificationHandler)

 public:
  static NotificationHandler* create(QObject* parent);

  virtual ~NotificationHandler();

 public slots:
  void showNotification();

 protected:
  explicit NotificationHandler(QObject* parent);

  virtual void notify(const QString& title, const QString& message,
                      int timerSec) = 0;

 private:
  QString m_switchingLocalizedServerCountry;
  QString m_switchingLocalizedServerCity;
  bool m_switching = false;

  // We want to show a 'disconnected' notification only if we were actually
  // connected.
  bool m_connected = false;
};

#endif  // NOTIFICATIONHANDLER_H
