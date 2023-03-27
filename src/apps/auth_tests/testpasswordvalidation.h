/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestPasswordValidation final : public QObject {
  Q_OBJECT

 public:
  explicit TestPasswordValidation(const QString& nonce) : m_nonce(nonce) {}

 private slots:
  void incrementalDecoder_data();
  void incrementalDecoder();

  void commonPasswords_data();
  void commonPasswords();

  void passwordLength_data();
  void passwordLength();

  void emailPassword();

 private:
  QString m_nonce;
};
