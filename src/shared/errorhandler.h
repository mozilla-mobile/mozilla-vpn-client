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
  // Note: Keep this in sync with ErrorTypeData in the cpp file.
  enum ErrorType {
    NoError,
    ConnectionFailureError,
    NoConnectionError,
    DependentConnectionError,
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
    CopiedToClipboardConfirmationAlert,
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

#define REPORTERROR(e, t) \
  ErrorHandler::instance()->errorHandle(e, t, __FILE__, __LINE__);

#define REPORTNETWORKERROR(e, p, t) \
  ErrorHandler::instance()->networkErrorHandle(e, p, t, __FILE__, __LINE__);

  static ErrorType toErrorType(QNetworkReply::NetworkError error);

  // Note: don't use this method directly. Use REPORTNETWORKERROR() instead.
  static void networkErrorHandle(QNetworkReply::NetworkError error,
                                 ErrorPropagationPolicy errorPropagationPolicy,
                                 const QString& taskName = QString(),
                                 const QString& fileName = QString(),
                                 int lineNumber = 0);

  ~ErrorHandler();

  static ErrorHandler* instance();

  AlertType alert() const { return m_alert; }

  // Note: don't use this method directly. Use REPORTERROR() instead.
  void errorHandle(ErrorType error, const QString& taskName = QString(),
                   const QString& fileName = QString(), int lineNumber = 0);

  Q_INVOKABLE void hideAlert() { setAlert(NoAlert); }
  Q_INVOKABLE void requestAlert(ErrorHandler::AlertType alert);

#define ERRORSTATE(name) \
  void name##Error();    \
  Q_SIGNAL void name();
#include "errorlist.h"
#undef ERRORSTATE

  static void registerCustomErrorHandler(ErrorType errorType,
                                         bool supportPolicyPropagation,
                                         AlertType (*getAlert)());

 signals:
  void alertChanged();
  void errorHandled();

 private:
  void setAlert(ErrorHandler::AlertType alert);

 private:
  AlertType m_alert = NoAlert;
  QTimer m_alertTimer;
};

#endif  // ERRORHANDLER_H
