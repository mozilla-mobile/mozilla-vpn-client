/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPPLISTENER_H
#define AUTHENTICATIONINAPPLISTENER_H

#include "authenticationlistener.h"

class AuthenticationInAppSession;

class AuthenticationInAppListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInAppListener)

 public:
  explicit AuthenticationInAppListener(QObject* parent);
  ~AuthenticationInAppListener();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod,
             const QString& emailAddress) override;

  void checkAccount(const QString& emailAddress);
  void setPassword(const QString& password);
  void verifyUnblockCode(const QString& unblockCode);
  void signIn(const QString& unblockCode = QString());
  void signUp();
  void verifySessionEmailCode(const QString& code);
  void resendVerificationSessionCodeEmail();
  void sendUnblockCodeEmail();
  void verifySessionTotpCode(const QString& code);

#ifdef UNIT_TEST
  void enableTotpCreation();
  void enableAccountDeletion();
#endif

  const QString& emailAddress() const { return m_emailAddress; }

  void aboutToFinish() override;

 private:
  void signInInternal(const QString& unblockCode);

  void processErrorCode(int errorCode);
  void processRequestFailure(QNetworkReply::NetworkError error,
                             const QByteArray& data);

  QByteArray generateAuthPw(const QString& password) const;

  void accountChecked(bool exists);
  void signInOrUpCompleted(const QString& sessionToken, bool accountVerified,
                           const QString& verificationMethod);
  void unblockCodeNeeded();
  void finalizeSignInOrUp();

#ifdef UNIT_TEST
  void createTotpCodes();
  void deleteAccount();
#endif
  void aboutToFinish() override;

 private:
  void fallbackRequired();

 private:
  AuthenticationInAppSession* m_session = nullptr;
  Task* m_task = nullptr;

  QString m_codeChallenge;
  QString m_codeChallengeMethod;

  QUrlQuery m_urlQuery;

  QString m_emailAddress;
  QByteArray m_authPw;

  QByteArray m_sessionToken;

#ifdef UNIT_TEST
  enum {
    OpNone,
    OpTotpCreationNeeded,
    OpAccountDeletionNeeded,
  } m_extraOp = OpNone;
#endif
};

#endif  // AUTHENTICATIONINAPPLISTENER_H
