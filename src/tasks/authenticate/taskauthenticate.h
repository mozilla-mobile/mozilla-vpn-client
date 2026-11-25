/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include <QByteArray>
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

  void handleDeepLink(const QUrl& url);
  void authenticatePkceSuccess(const QString& code);

  // Finishes a flow started by a previous process. Must be called before run().
  void resumeWithCode(const QString& code, const QByteArray& codeVerifier);

  // Returns and clears the saved verifier or empty if missing or expired.
  static QByteArray consumePendingCodeVerifier();

 signals:
  void authenticationAborted();
  void authenticationStarted();
  void authenticationCompleted(const QByteArray& json, const QString& token);

 private:
  void authenticationCompletedInternal(const QByteArray& data);
  static void clearPendingCodeVerifier();

 private:
  AuthenticationListener* m_authenticationListener = nullptr;
  AuthenticationListener::AuthenticationType m_authenticationType =
      AuthenticationListener::AuthenticationInBrowser;

  QByteArray m_pkceCodeVerifier;
  QString m_resumeCode;
};

#endif  // TASKAUTHENTICATE_H
