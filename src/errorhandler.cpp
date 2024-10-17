/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "errorhandler.h"

#include <QApplication>

#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
using namespace std::chrono_literals;
constexpr const std::chrono::seconds HIDE_ALERT_DELAY = 4s;

ErrorHandler* s_instance = nullptr;
Logger logger("ErrorHandler");

struct ErrorTypeData {
  ErrorTypeData(ErrorHandler::ErrorType errorType,
                bool supportPolicyPropagation,
                ErrorHandler::AlertType (*getAlert)())
      : m_errorType(errorType),
        m_supportPolicyPropagation(supportPolicyPropagation),
        m_getAlert(getAlert) {}

  ErrorHandler::ErrorType m_errorType = ErrorHandler::NoError;
  bool m_supportPolicyPropagation = false;

  ErrorHandler::AlertType (*m_getAlert)() = nullptr;
};

QList<ErrorTypeData> s_errorData{
    ErrorTypeData(ErrorHandler::NoError, true,
                  []() { return ErrorHandler::NoAlert; }),

    ErrorTypeData(ErrorHandler::ConnectionFailureError, true,
                  []() { return ErrorHandler::ConnectionFailedAlert; }),

    ErrorTypeData(ErrorHandler::NoConnectionError, true,
                  []() { return ErrorHandler::ConnectionFailedAlert; }),

    ErrorTypeData(ErrorHandler::DependentConnectionError, true,
                  []() { return ErrorHandler::ConnectionFailedAlert; }),

    ErrorTypeData(ErrorHandler::AuthenticationError, false,
                  []() { return ErrorHandler::AuthenticationFailedAlert; }),

    ErrorTypeData(ErrorHandler::ControllerError, true,
                  []() { return ErrorHandler::ControllerErrorAlert; }),
    ErrorTypeData(ErrorHandler::SplitTunnelError, true,
                  []() { return ErrorHandler::SplitTunnelErrorAlert; }),

    ErrorTypeData(ErrorHandler::RemoteServiceError, true,
                  []() { return ErrorHandler::RemoteServiceErrorAlert; }),

    ErrorTypeData(ErrorHandler::SubscriptionFailureError, false,
                  []() { return ErrorHandler::SubscriptionFailureAlert; }),

    ErrorTypeData(ErrorHandler::GeoIpRestrictionError, false,
                  []() { return ErrorHandler::GeoIpRestrictionAlert; }),

    ErrorTypeData(ErrorHandler::UnrecoverableError, true,
                  []() { return ErrorHandler::UnrecoverableErrorAlert; }),

    ErrorTypeData(ErrorHandler::IgnoredError, true,
                  []() { return ErrorHandler::NoAlert; }),
};

const ErrorTypeData* findError(ErrorHandler::ErrorType error) {
  for (const ErrorTypeData& errorData : s_errorData) {
    if (errorData.m_errorType == error) {
      return &errorData;
    }
  }
  Q_ASSERT(false);
  return nullptr;
}

}  // namespace

// static
ErrorHandler* ErrorHandler::instance() {
  if (!s_instance) {
    s_instance = new ErrorHandler(qApp);
  };
  return s_instance;
}

ErrorHandler::ErrorHandler(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(ErrorHandler);

  connect(&m_alertTimer, &QTimer::timeout, this,
          [this]() { setAlert(NoAlert); });
}

ErrorHandler::~ErrorHandler() { MZ_COUNT_DTOR(ErrorHandler); }

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
      return DependentConnectionError;
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

void ErrorHandler::errorHandle(ErrorHandler::ErrorType error,
                               const QString& taskName, const QString& fileName,
                               int lineNumber) {
  logger.debug() << "Handling error" << error;

  const ErrorTypeData* errorData = findError(error);
  Q_ASSERT(errorData);

  AlertType alert = errorData->m_getAlert();
  logger.error() << "Alert:" << alert;

  if (alert == NoAlert) {
    return;
  }

  setAlert(alert);

  QVariantMap extraKeys;
  mozilla::glean::sample::ErrorAlertShownExtra extras;

  if (!taskName.isEmpty()) {
    extraKeys["task"] = taskName;
    extras._task = taskName;
  }

  if (!fileName.isEmpty()) {
    extraKeys["filename"] = fileName;
    extras._filename = fileName;
  }

  if (lineNumber >= 0) {
    extraKeys["linenumber"] = lineNumber;
    extras._linenumber = lineNumber;
  }

  mozilla::glean::sample::error_alert_shown.record(extras);

  emit errorHandled();
}

void ErrorHandler::requestAlert(AlertType alert) {
  // For now, let's keep it simple. In the future we can apply filters here.
  setAlert(alert);
}

void ErrorHandler::setAlert(AlertType alert) {
  m_alertTimer.stop();

  if (alert != NoAlert) {
    m_alertTimer.start(HIDE_ALERT_DELAY);
  }

  m_alert = alert;
  emit alertChanged();
}

// static
void ErrorHandler::networkErrorHandle(
    QNetworkReply::NetworkError error,
    ErrorPropagationPolicy errorPropagationPolicy, const QString& taskName,
    const QString& fileName, int lineNumber) {
  ErrorHandler::ErrorType errorType = toErrorType(error);

  const ErrorTypeData* errorData = findError(errorType);
  Q_ASSERT(errorData);

  if (!errorData->m_supportPolicyPropagation ||
      errorPropagationPolicy == PropagateError) {
    ErrorHandler::instance()->errorHandle(errorType, taskName, fileName,
                                          lineNumber);
  }
}

// static
void ErrorHandler::registerCustomErrorHandler(ErrorType errorType,
                                              bool supportPolicyPropagation,
                                              AlertType (*getAlert)()) {
  s_errorData.prepend(
      ErrorTypeData(errorType, supportPolicyPropagation, getAlert));
}
