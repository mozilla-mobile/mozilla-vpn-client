/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APIERROR_H
#define APIERROR_H

#include <QString>
#include <QObject>

class QByteArray;

class ApiError final {
  Q_GADGET

 public:
  ApiError() {};
  ApiError(const ApiError& other);
  ApiError& operator=(const ApiError& other);

  // Error codes transcribed from guardian-website/server/lib/errors.ts
  enum ErrorNumbers : int {
    NoError = 0,

    MissingPubkeyError = 100,
    MissingNameError = 101,
    InvalidPubkeyError = 102,
    PubkeyAlreadyUsed = 103,
    KeyLimitReachedError = 104,
    PubkeyNotFound = 105,
    SecretKeyNotFound = 106,

    FxaUnauthorizedForRouteError = 110,

    InvalidToken = 120,
    UserNotFound = 121,
    DeviceNotFound = 122,
    NoActiveSubsription = 123,
    MissingAuthCodeError = 127,
    MissingAuthCodeVerifierError = 128,
    AuthCodeExpiredError = 129,
    InvalidAuthCodeVerifierError = 130,
    MissingReceiptError = 141,
    UserAlreadyHasActiveSubscription = 143,
    ReceiptIsBeingUsed = 145,
    InvalidTokenError = 147,
    InvalidSkuError = 148,
    MissingTokenError = 149,
    MissingSkuError = 150,
    FxaTokenValidationError = 151,
    FxaPlansError = 152,
    FxaSupportTicketError = 153,
    FxaSupportTicketNotReturned = 154,
    SupportTicketIssueTextSizeError = 155,
    SupportTicketMissingLogsError = 157,
    MissingAccessToken = 159,
    AdjustMissingOrInvalidHeaders = 161,
    AdjustMissingOrInvalidPath = 162,
    FxaScopesNotAllowedError = 163,
    MissingOriginalTransactionIdError = 164,
    MissingExperimenterIdOrAuthHeader = 165,
  };
  Q_ENUM(ErrorNumbers);

  bool fromJson(const QByteArray& json);

  int code() const { return m_code; }
  int errnum() const { return m_errnum; }
  const QString& message() const { return m_message; }
  const QString& error() const { return m_message; }

 private:
  int m_code = 0;
  int m_errnum = 0;
  QString m_message;
};

#endif  // APIERROR_H
