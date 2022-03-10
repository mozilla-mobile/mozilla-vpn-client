/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestSignUpAndIn final : public QObject {
  Q_OBJECT

 public:
  TestSignUpAndIn(const QString& pattern, bool totpCreation = false);
  ~TestSignUpAndIn() = default;

 private slots:
  void signUp();
  void signIn();

 private:
  QString fetchSessionCode();
  QString fetchUnblockCode();
  QString fetchCode(const QString& code);
  void waitForTotpCodes();

  QString m_emailAccount;
  bool m_totpCreation = false;

  // We want to send the wrong totp code only once. At the first
  // StateVerificationSessionByTotpNeeded state change.
  bool m_sendWrongTotpCode = true;

  QString m_totpSecret;
};
