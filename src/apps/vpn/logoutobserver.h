/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGOUTOBSERVER_H
#define LOGOUTOBSERVER_H

#include <QObject>
/**
 * @brief Observes the User State.
 *
 * Will fire the event ready(), whenever
 * a user session ended.
 * After that happens the Observer will delete itself.
 */
class LogoutObserver final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LogoutObserver)

 public:
  explicit LogoutObserver(QObject* parent);
  ~LogoutObserver();

 signals:
  void ready();

 private slots:
  void userStateChanged();
};

#endif  // LOGOUTOBSERVER_H
