/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include <QUuid>

#include "authenticationlistener.h"
#include "task.h"

class QByteArray;

class TaskAuthenticate final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskAuthenticate)

 public:
  explicit TaskAuthenticate(
      AuthenticationListener::AuthenticationType authenticationType);
  ~TaskAuthenticate();

  void run() override;

 signals:
  void authenticationAborted();
  void authenticationCompleted(const QByteArray& json, const QString& token);

 private:
  void authenticationCompletedInternal(const QByteArray& data);

 private:
  AuthenticationListener* m_authenticationListener = nullptr;
  AuthenticationListener::AuthenticationType m_authenticationType =
      AuthenticationListener::AuthenticationInBrowser;
  QUuid m_metricUuid = QUuid::createUuid();
};

#endif  // TASKAUTHENTICATE_H
