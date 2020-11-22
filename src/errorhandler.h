/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QNetworkReply>

class ErrorHandler final {
 public:
  enum ErrorType {
    NoError,
    ConnectionFailureError,
    NoConnectionError,
    AuthenticationError,
    BackendServiceError,
    IgnoredError,
  };

  static ErrorType toErrorType(QNetworkReply::NetworkError error);
};

#endif  // ERRORHANDLER_H
