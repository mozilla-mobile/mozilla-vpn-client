/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPPLISTENER_H
#define AUTHENTICATIONINAPPLISTENER_H

#include "authenticationlistener.h"

#include <QNetworkReply>

class AuthenticationInAppListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInAppListener)

 public:
  explicit AuthenticationInAppListener(QObject* parent);
  ~AuthenticationInAppListener();

  void reset();

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
  void allowUpperCaseEmailAddress();
  void enableAccountDeletion();
#endif

  const QString& emailAddress() const { return m_emailAddress; }

  void aboutToFinish() override;

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
  void deleteAccount();
#endif

 private:
  Task* m_task = nullptr;

  QString m_codeChallenge;
  QString m_codeChallengeMethod;

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

#ifdef UNIT_TEST
  bool m_allowUpperCaseEmailAddress = false;
  enum {
    OpNone,
    OpTotpCreationNeeded,
    OpAccountDeletionNeeded,
  } m_extraOp = OpNone;
#endif
};

#endif  // AUTHENTICATIONINAPPLISTENER_H
