/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include "errorhandler.h"

constexpr const char* CODE_CHALLENGE_METHOD = "S256";

class Task;

class AuthenticationListener : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationListener)

 public:
  enum AuthenticationType {
    AuthenticationInBrowser,
    AuthenticationInApp,
  };

  static AuthenticationListener* create(QObject* parent,
                                        AuthenticationType authenticationType);

  virtual void start(Task* task, const QString& codeChallenge,
                     const QString& codeChallengeMethod,
                     const QString& emailAddress = QString()) = 0;

  static QUrl createAuthenticationUrl(const QString& codeChallenge,
                                      const QString& codeChallengeMethod,
                                      const QString& emailAddress);

  static QUrl createLoginVerifyUrl();

  static void generatePkceCodes(QByteArray& pkceCodeVerifier,
                                QByteArray& pkceCodeChallenge);

  virtual void aboutToFinish();

 signals:
  void completed(const QString& code);

  void failed(ErrorHandler::ErrorType error);

  void abortedByUser();

  void readyToFinish();

 protected:
  explicit AuthenticationListener(QObject* parent);
  virtual ~AuthenticationListener();
};

#endif  // AUTHENTICATIONLISTENER_H
