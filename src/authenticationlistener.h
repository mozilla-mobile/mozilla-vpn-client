/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include "errorhandler.h"
#include "mozillavpn.h"

#include <QObject>

class QUrl;
class QUrlQuery;

class AuthenticationListener : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationListener)

 public:
  static AuthenticationListener* create(
      QObject* parent, MozillaVPN::AuthenticationType authenticationType);

  virtual void start(MozillaVPN* vpn, QUrl& url, QUrlQuery& query) = 0;

 signals:
  void completed(const QString& code);

  void failed(ErrorHandler::ErrorType error);

  void abortedByUser();

 protected:
  explicit AuthenticationListener(QObject* parent);
  virtual ~AuthenticationListener();
};

#endif  // AUTHENTICATIONLISTENER_H
