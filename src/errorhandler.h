/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class ErrorHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ErrorHandler)

  Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)

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
  Q_ENUM(ErrorType)

  enum AlertType {
    NoAlert,
    AuthenticationFailedAlert,
    ConnectionFailedAlert,
    LogoutAlert,
    NoConnectionAlert,
    ControllerErrorAlert,
    RemoteServiceErrorAlert,
    SubscriptionFailureAlert,
    GeoIpRestrictionAlert,
    UnrecoverableErrorAlert,
    AuthCodeSentAlert,
  };
  Q_ENUM(AlertType)

  enum ErrorPropagationPolicy {
    // Do not propagate the error up to the frontend code. The error will be
    // logged but it will not be shown to the user.
    DoNotPropagateError,

    // The error needs to be propagated through the frontend code and shown to
    // the user as an alert or something else.
    PropagateError,
  };
  Q_ENUM(ErrorPropagationPolicy);

  static ErrorType toErrorType(QNetworkReply::NetworkError error);

  static void networkErrorHandle(
      QNetworkReply::NetworkError error,
      ErrorPropagationPolicy errorPropagationPolicy = PropagateError);

  ~ErrorHandler();

  static ErrorHandler* instance();

  AlertType alert() const { return m_alert; }

  void errorHandle(ErrorType error);

  void hideAlert() { setAlert(NoAlert); }
  Q_INVOKABLE void setAlert(ErrorHandler::AlertType alert);

#define ERRORSTATE(name) \
  void name##Error();    \
  Q_SIGNAL void name();
#include "errorlist.h"
#undef ERRORSTATE

 signals:
  void alertChanged();

 private:
  AlertType m_alert = NoAlert;
  QTimer m_alertTimer;
};

#endif  // ERRORHANDLER_H
