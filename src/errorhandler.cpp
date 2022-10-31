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

void ErrorHandler::errorHandle(ErrorHandler::ErrorType error) {
  logger.debug() << "Handling error" << error;

  Q_ASSERT(error != ErrorHandler::NoError);

  AlertType alert = NoAlert;

  MozillaVPN* vpn = MozillaVPN::instance();

  switch (error) {
    case ErrorHandler::VPNDependentConnectionError:
      if (vpn->controller()->state() == Controller::State::StateOn ||
          vpn->controller()->state() == Controller::State::StateConfirming) {
        // connection likely isn't stable yet
        logger.error() << "Ignore network error probably caused by enabled VPN";
        return;
      } else if (vpn->controller()->state() == Controller::State::StateOff) {
        // We are off, so this means a request failed, not the
        // VPN. Change it to No Connection
        alert = NoConnectionAlert;
        break;
      }
      [[fallthrough]];
    case ErrorHandler::ConnectionFailureError:
      alert = ConnectionFailedAlert;
      break;

    case ErrorHandler::NoConnectionError:
      if (vpn->connectionHealth() && vpn->connectionHealth()->isUnsettled()) {
        return;
      }
      alert = NoConnectionAlert;
      break;

    case ErrorHandler::AuthenticationError:
      alert = AuthenticationFailedAlert;
      break;

    case ErrorHandler::ControllerError:
      alert = ControllerErrorAlert;
      break;

    case ErrorHandler::RemoteServiceError:
      alert = RemoteServiceErrorAlert;
      break;

    case ErrorHandler::SubscriptionFailureError:
      alert = SubscriptionFailureAlert;
      break;

    case ErrorHandler::GeoIpRestrictionError:
      alert = GeoIpRestrictionAlert;
      break;

    case ErrorHandler::UnrecoverableError:
      alert = UnrecoverableErrorAlert;
      break;

    default:
      break;
  }

  setAlert(alert);

  logger.error() << "Alert:" << alert << "State:" << vpn->state();

  if (alert == NoAlert) {
    return;
  }

  // Any error in authenticating state sends to the Initial state.
  if (vpn->state() == MozillaVPN::StateAuthenticating) {
    if (alert == GeoIpRestrictionAlert) {
      emit vpn->recordGleanEvent(GleanSample::authenticationFailureByGeo);
    } else {
      emit vpn->recordGleanEvent(GleanSample::authenticationFailure);
    }
    vpn->reset(true);
    return;
  }

  if (alert == AuthenticationFailedAlert) {
    vpn->reset(true);
    return;
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

// static
void ErrorHandler::networkErrorHandle(
    QNetworkReply::NetworkError error,
    ErrorPropagationPolicy errorPropagationPolicy) {
  if (errorPropagationPolicy == PropagateError) {
    ErrorHandler::instance()->errorHandle(toErrorType(error));
  }
}
