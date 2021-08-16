/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include "errorhandler.h"
#include "mozillavpn.h"

#include <QObject>
#include <QUrl>

class AuthenticationListener : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationListener)

 public:
  static AuthenticationListener* create(
      QObject* parent, MozillaVPN::AuthenticationType authenticationType);

  virtual void start(const QString& codeChallenge,
                     const QString& codeChallengeMethod,
                     const QString& emailAddress = QString()) = 0;

  static QUrl createAuthenticationUrl(
      MozillaVPN::AuthenticationType authenticationType,
      const QString& codeChallenge, const QString& codeChallengeMethod,
      const QString& emailAddress);

 signals:
  void completed(const QString& code);

  void failed(ErrorHandler::ErrorType error);

  void abortedByUser();

 protected:
  explicit AuthenticationListener(QObject* parent);
  virtual ~AuthenticationListener();
};

#endif  // AUTHENTICATIONLISTENER_H
