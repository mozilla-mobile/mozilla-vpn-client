/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"

#include <QApplication>

// in seconds, hide alerts
constexpr const uint32_t HIDE_ALERT_SEC = 4;

namespace {
ErrorHandler* s_instance = nullptr;
Logger logger(LOG_MAIN, "ErrorHandler");

struct ErrorData {
  ErrorHandler::ErrorType m_error;
  ErrorHandler::AlertType (*m_alert)();
  bool (*m_resetNeeded)();

  ErrorData(
      ErrorHandler::ErrorType error, ErrorHandler::AlertType (*alert)(),
      bool (*resetNeeded)() = []() -> bool {
        // By default, any error in authenticating state sends to the Initial
        // state.
        return MozillaVPN::instance()->state() ==
               MozillaVPN::StateAuthenticating;
      })
      : m_error(error), m_alert(alert), m_resetNeeded(resetNeeded) {}
};

// Important: keep this enum in sync with the `ErrorType` enum in
// errorhandler.h
ErrorData s_errors[] = {
    ErrorData(
        ErrorHandler::AuthenticationError,
        []() -> ErrorHandler::AlertType {
          return ErrorHandler::AuthenticationFailedAlert;
        },
        []() -> bool { return true; }),

    ErrorData(ErrorHandler::ConnectionFailureError,
              []() -> ErrorHandler::AlertType {
                MozillaVPN* vpn = MozillaVPN::instance();
                Q_ASSERT(vpn);

                if (vpn->state() == MozillaVPN::StateMain &&
                        (vpn->controller()->state() != Controller::StateOn) ||
                    (vpn->connectionHealth() &&
                     vpn->connectionHealth()->isUnsettled())) {
                  return ErrorHandler::NoAlert;
                }

                return ErrorHandler::ConnectionFailedAlert;
              }),

    ErrorData(ErrorHandler::ControllerError,
              []() -> ErrorHandler::AlertType {
                return ErrorHandler::ControllerErrorAlert;
              }),

    ErrorData(ErrorHandler::GeoIpRestrictionError,
              []() -> ErrorHandler::AlertType {
                return ErrorHandler::GeoIpRestrictionAlert;
              }),

    ErrorData(
        ErrorHandler::IgnoredError,
        []() -> ErrorHandler::AlertType { return ErrorHandler::NoAlert; }),

    ErrorData(ErrorHandler::NoConnectionError,
              []() -> ErrorHandler::AlertType {
                MozillaVPN* vpn = MozillaVPN::instance();
                Q_ASSERT(vpn);

                if (vpn->state() == MozillaVPN::StateMain &&
                        (vpn->controller()->state() != Controller::StateOn) ||
                    (vpn->connectionHealth() &&
                     vpn->connectionHealth()->isUnsettled())) {
                  return ErrorHandler::NoAlert;
                }

                return ErrorHandler::NoConnectionAlert;
              }),

    ErrorData(
        ErrorHandler::NoError,
        []() -> ErrorHandler::AlertType { return ErrorHandler::NoAlert; }),

    ErrorData(ErrorHandler::RemoteServiceError,
              []() -> ErrorHandler::AlertType {
                return ErrorHandler::RemoteServiceErrorAlert;
              }),

    ErrorData(ErrorHandler::SubscriptionFailureError,
              []() -> ErrorHandler::AlertType {
                return ErrorHandler::SubscriptionFailureAlert;
              }),

    ErrorData(ErrorHandler::UnrecoverableError,
              []() -> ErrorHandler::AlertType {
                return ErrorHandler::UnrecoverableErrorAlert;
              }),
};

}  // namespace

// static
ErrorHandler* ErrorHandler::instance() {
  if (!s_instance) {
    s_instance = new ErrorHandler(qApp);
  };
  return s_instance;
}

ErrorHandler::ErrorHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(ErrorHandler);

#if defined(MVPN_DEBUG) || defined(UNIT_TEST)
  // Let's see if the ErrorType are in sync with the ErrorData objects
  for (int i = 0; i < (sizeof(s_errors) / sizeof(s_errors[0])); ++i) {
    Q_ASSERT(s_errors[i].m_error == (ErrorType)i);
  }
#endif

  connect(&m_alertTimer, &QTimer::timeout, this,
          [this]() { setAlert(NoAlert); });
}

ErrorHandler::~ErrorHandler() { MVPN_COUNT_DTOR(ErrorHandler); }

#define ERRORSTATE(name)                           \
  void ErrorHandler::name##Error() {               \
    logger.warning() << #name << " error handled"; \
    emit name();                                   \
  }

#include "errorlist.h"
#undef ERRORSTATE

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
      [[fallthrough]];
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

void ErrorHandler::errorHandle(ErrorHandler::ErrorType error) {
  logger.debug() << "Handling error" << error;

  Q_ASSERT(error < (sizeof(s_errors) / sizeof(s_errors[0])));
  ErrorData& ed = s_errors[error];
  Q_ASSERT(ed.m_error == error);

  AlertType alert = ed.m_alert();
  if (alert == NoAlert) {
    return;
  }

  setAlert(alert);

  if (ed.m_resetNeeded()) {
    MozillaVPN::instance()->reset(true);
  }
}

void ErrorHandler::setAlert(AlertType alert) {
  m_alertTimer.stop();

  if (alert != NoAlert) {
    m_alertTimer.start(1000 * HIDE_ALERT_SEC);
  }

  m_alert = alert;
  emit alertChanged();
}
