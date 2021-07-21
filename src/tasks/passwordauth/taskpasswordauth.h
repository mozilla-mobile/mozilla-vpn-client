/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKPASSWORDAUTH_H
#define TASKPASSWORDAUTH_H

#include "networkrequest.h"
#include "errorhandler.h"
#include "task.h"

class TaskPasswordAuth final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskPasswordAuth)

 public:
  TaskPasswordAuth(const QString& username, const QString& password);
  ~TaskPasswordAuth();

  void run(MozillaVPN* vpn) override;

 private:
  void vpnLoginRedirect(NetworkRequest* request);
  void fxaLoginComplete(const QByteArray& data);
  void fxaAuthzComplete(const QByteArray& data);
  void vpnVerifyComplete(const QByteArray& data);

  void authFailed(ErrorHandler::ErrorType error);
  void netRequestFailed(QNetworkReply::NetworkError error,
                        const QByteArray& data);

 private:
  QString m_username;
  QMap<QString, QString> m_querydata;
  QByteArray m_authpw;
  QByteArray m_session;
  QByteArray m_verifier;
};

#endif  // TASKPASSWORDAUTH_H
