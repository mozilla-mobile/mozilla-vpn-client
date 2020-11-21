/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONLISTENER_H
#define AUTHENTICATIONLISTENER_H

#include "errorhandler.h"

#include <QObject>

class MozillaVPN;
class QOAuthHttpServerReplyHandler;
class QUrl;
class QUrlQuery;

class AuthenticationListener final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationListener)

 public:
  static AuthenticationListener* create(QObject* parent);

  ~AuthenticationListener();

  void start(MozillaVPN* vpn, QUrl& url, QUrlQuery& query);

 private:
  explicit AuthenticationListener(QObject* parent);

 signals:
  void completed(const QString& code);

  void failed(ErrorHandler::ErrorType error);

  void abortedByUser();

 private:
  QOAuthHttpServerReplyHandler* m_server = nullptr;
};

#endif  // AUTHENTICATIONLISTENER_H
