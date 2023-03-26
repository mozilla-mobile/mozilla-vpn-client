/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestSignUpAndIn final : public QObject {
  Q_OBJECT

 public:
  // We want to send the wrong totp code a few times using strings, numbers,
  // etc. at the first StateVerificationSessionByTotpNeeded state change.
  enum SendTotpCodeState {
    NoCodeSent = 0,
    SendWrongTotpCodeNumber,
    SendWrongTotpCodeString,
    SendWrongTotpCodeAlphaNumeric,
    GoodTotpCode,
  };
  Q_ENUM(SendTotpCodeState);

  TestSignUpAndIn(const QString& nonce, const QString& pattern,
                  bool totpCreation = false);
  ~TestSignUpAndIn() = default;

 private slots:
  void signInWithError();
  void signUp();
  void signUpWithError();
  void signIn();
  void deleteAccount();

 private:
  QString fetchSessionCode();
  QString fetchUnblockCode();
  QString fetchCode(const QString& code);
  void waitForTotpCodes();
  void fetchAndSendUnblockCode();
  void sendNextTotpCode();

  QString m_emailAccount;
  bool m_totpCreation = false;

  SendTotpCodeState m_sendTotpCodeState = NoCodeSent;

  QString m_totpSecret;
};
