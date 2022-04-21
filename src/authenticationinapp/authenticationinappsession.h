/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPPSESSION_H
#define AUTHENTICATIONINAPPSESSION_H

#include "errorhandler.h"

#include <QObject>
#include <QNetworkReply>

class Task;

class AuthenticationInAppSession final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInAppSession)

 public:
  AuthenticationInAppSession(QObject* parent);
  ~AuthenticationInAppSession();

  void reset();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod, const QString& emailAddress);

  void checkAccount(const QString& emailAddress);
  void setPassword(const QString& password);
  void verifyUnblockCode(const QString& unblockCode);
  void signIn(const QString& unblockCode = QString());
  void signUp();
  void verifySessionEmailCode(const QString& code);
  void resendVerificationSessionCodeEmail();
  void sendUnblockCodeEmail();
  void verifySessionTotpCode(const QString& code);
  void startAccountDeletionFlow();
  void deleteAccount();

#ifdef UNIT_TEST
  void enableTotpCreation();
  void allowUpperCaseEmailAddress();
#endif

  const QString& emailAddress() const { return m_emailAddress; }
  const QStringList& attachedClients() const { return m_attachedClients; }

  void terminate();

 signals:
  void completed(const QString& code);
  void failed(ErrorHandler::ErrorType error);
  void accountDeleted();
  void fallbackRequired();
  void terminated();

 private:
  void signInInternal(const QString& unblockCode);

  void processErrorObject(const QJsonObject& obj);
  void processRequestFailure(QNetworkReply::NetworkError error,
                             const QByteArray& data);

  QByteArray generateAuthPw() const;

  void accountChecked(bool exists);
  void signInOrUpCompleted(const QString& sessionToken, bool accountVerified,
                           const QString& verificationMethod);
  void unblockCodeNeeded();
  void finalizeSignInOrUp();

#ifdef UNIT_TEST
  void createTotpCodes();
#endif

 private:
  Task* m_task = nullptr;

  struct {
    QString m_clientId;
    QString m_deviceId;
    QString m_state;
    QString m_scope;
    QString m_accessType;
    QString m_flowId;
    double m_flowBeginTime;
  } m_fxaParams;

  QString m_emailAddress;
  QString m_password;

  // FxA can return a different case format for the email address. Usually,
  // this is equal to `m_emailAddress`.
  QString m_emailAddressCaseFix;

  QByteArray m_sessionToken;

  QStringList m_attachedClients;

#ifdef UNIT_TEST
  bool m_allowUpperCaseEmailAddress = false;
  enum {
    OpNone,
    OpTotpCreationNeeded,
  } m_extraOp = OpNone;
#endif
};

#endif  // AUTHENTICATIONINAPPSESSION_H
