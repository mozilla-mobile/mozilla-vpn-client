/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>
#include <QNetworkReply>

class ErrorHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ErrorHandler)

 private:
  explicit ErrorHandler(QObject* parent);

 public:
  enum ErrorType {
    NoError,
    ConnectionFailureError,
    NoConnectionError,
    VPNDependentConnectionError,
    AuthenticationError,
    ControllerError,
    RemoteServiceError,
    SubscriptionFailureError,
    GeoIpRestrictionError,
    UnrecoverableError,
    IgnoredError,
  };

  static ErrorType toErrorType(QNetworkReply::NetworkError error);

  ~ErrorHandler();

  static ErrorHandler* instance();

#define ERRORSTATE(name) \
  void name##Error();    \
  Q_SIGNAL void name();
#include "errorlist.h"
#undef ERRORSTATE
};

#endif  // ERRORHANDLER_H
