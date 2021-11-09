/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
ErrorHandler* s_instance = nullptr;
Logger logger(LOG_MAIN, "ErrorHandler");
}  // namespace

// static
ErrorHandler* ErrorHandler::instance() {
  if (!s_instance) {
    s_instance = new ErrorHandler();
  };
  return s_instance;
}

ErrorHandler::ErrorHandler() { MVPN_COUNT_CTOR(ErrorHandler); }

ErrorHandler::~ErrorHandler() { MVPN_COUNT_DTOR(ErrorHandler); }

#define ERROR(name)                                \
  void ErrorHandler::name##Error() {               \
    logger.warning() << #name << " error handled"; \
    emit name();                                   \
  }

#include "errorlist.h"
#undef ERROR

// static
ErrorHandler::ErrorType ErrorHandler::toErrorType(
    QNetworkReply::NetworkError error) {
  switch (error) {
    case QNetworkReply::ConnectionRefusedError:
      [[fallthrough]];
    case QNetworkReply::RemoteHostClosedError:
      [[fallthrough]];
    case QNetworkReply::SslHandshakeFailedError:
      [[fallthrough]];
    case QNetworkReply::TemporaryNetworkFailureError:
      [[fallthrough]];
    case QNetworkReply::NetworkSessionFailedError:
      [[fallthrough]];
    case QNetworkReply::TooManyRedirectsError:
      [[fallthrough]];
    case QNetworkReply::InsecureRedirectError:
      [[fallthrough]];
    case QNetworkReply::ProxyConnectionRefusedError:
      [[fallthrough]];
    case QNetworkReply::ProxyConnectionClosedError:
      [[fallthrough]];
    case QNetworkReply::ProxyNotFoundError:
      [[fallthrough]];
    case QNetworkReply::ProxyTimeoutError:
      [[fallthrough]];
    case QNetworkReply::ProxyAuthenticationRequiredError:
      [[fallthrough]];
    case QNetworkReply::ServiceUnavailableError:
      return ConnectionFailureError;

    case QNetworkReply::HostNotFoundError:
      return NoConnectionError;
    case QNetworkReply::TimeoutError:
      return VPNDependentConnectionError;
    case QNetworkReply::UnknownNetworkError:
      // On mac, this means: no internet
      // On Android check if
      // openSSL is missing.
      return NoConnectionError;

    case QNetworkReply::OperationCanceledError:
      [[fallthrough]];
    case QNetworkReply::BackgroundRequestNotAllowedError:
      [[fallthrough]];
    case QNetworkReply::ContentAccessDenied:
      [[fallthrough]];
    case QNetworkReply::ContentNotFoundError:
      [[fallthrough]];
    case QNetworkReply::ContentReSendError:
      [[fallthrough]];
    case QNetworkReply::ContentConflictError:
      [[fallthrough]];
    case QNetworkReply::ContentGoneError:
      [[fallthrough]];
    case QNetworkReply::InternalServerError:
      [[fallthrough]];
    case QNetworkReply::OperationNotImplementedError:
      [[fallthrough]];
    case QNetworkReply::ProtocolUnknownError:
      [[fallthrough]];
    case QNetworkReply::ProtocolInvalidOperationError:
      [[fallthrough]];
    case QNetworkReply::UnknownProxyError:
      [[fallthrough]];
    case QNetworkReply::UnknownContentError:
      [[fallthrough]];
    case QNetworkReply::ProtocolFailure:
      [[fallthrough]];
    case QNetworkReply::UnknownServerError:
      return IgnoredError;

    case QNetworkReply::ContentOperationNotPermittedError:
      [[fallthrough]];
    case QNetworkReply::AuthenticationRequiredError:
      return AuthenticationError;

    case QNetworkReply::NoError:
      return NoError;
  }

  return IgnoredError;
}
