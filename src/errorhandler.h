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

  ErrorHandler();
  ~ErrorHandler();

  static ErrorHandler* instance();

#define ERROR(name) void name##Error();

#include "errorlist.h"
#undef ERROR

 signals:
#define ERROR(name) void name();

#include "errorlist.h"
#undef ERROR
};

#endif  // ERRORHANDLER_H
