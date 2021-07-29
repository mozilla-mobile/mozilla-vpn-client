/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPPLISTENER_H
#define AUTHENTICATIONINAPPLISTENER_H

#include "authenticationlistener.h"

#include <QNetworkReply>
#include <QUrlQuery>

class AuthenticationInAppListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInAppListener)

 public:
  explicit AuthenticationInAppListener(QObject* parent);
  ~AuthenticationInAppListener();

  void start(MozillaVPN* vpn, QUrl& url, QUrlQuery& query) override;

  void checkAccount(const QString& emailAddress);
  void setPassword(const QString& password);
  void signIn(const QString& verificationCode = QString());
  void signUp();
  void verifyEmailCode(const QString& code);
  void verifySessionEmailCode(const QString& code);
  void resendVerificationSessionCodeEmail();
  void verifySessionTotpCode(const QString& code);

 private:
  void processErrorCode(int errorCode);
  void processRequestFailure(QNetworkReply::NetworkError error,
                             const QByteArray& data);

  QByteArray generateAuthPw(const QString& password) const;

  void accountChecked(bool exists);
  void signInCompleted(const QString& sessionToken, bool accountVerified,
                       const QString& verificationMethod);
  void emailVerificationNeeded();
  void finalizeSignIn();

 private:
  QString m_codeChallenge;
  QUrlQuery m_urlQuery;

  QString m_emailAddress;
  QByteArray m_authPw;

  QByteArray m_sessionToken;
};

#endif  // AUTHENTICATIONINAPPLISTENER_H
