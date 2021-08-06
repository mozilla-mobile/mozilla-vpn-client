/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestSignUpAndIn final : public QObject {
  Q_OBJECT

 public:
  TestSignUpAndIn(const QString& pattern);
  ~TestSignUpAndIn() = default;

 private slots:
  void signUp();
  void signIn();

 private:
  QString fetchSessionCode();
  QString fetchUnblockCode();
  QString fetchCode(const QString& code);

  QString m_emailAccount;
};
