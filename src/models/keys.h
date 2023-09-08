/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef KEYS_H
#define KEYS_H

#include <QObject>
#include <QString>

class Keys final {
  Q_DISABLE_COPY_MOVE(Keys)

 public:
  Keys();
  ~Keys();

  [[nodiscard]] bool fromSettings();

  bool initialized() const {
    return !m_privateKey.isEmpty() && !m_publicKey.isEmpty();
  }

  void storeKeys(const QString& privateKey, const QString& publicKey);

  void forgetKeys();

  const QString& privateKey() const { return m_privateKey; }

  const QString& publicKey() const { return m_publicKey; }

 private:
  QString m_privateKey;
  QString m_publicKey;
};

#endif  // KEYS_H
