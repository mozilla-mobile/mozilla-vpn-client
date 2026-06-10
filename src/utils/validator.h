/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QObject>
#include <QUrl>

class Validator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Validator)

 public:
  static Validator* instance();
  Q_INVOKABLE static bool validateEmailAddress(const QString& emailAddress);

 private:
  explicit Validator(QObject* parent);
  ~Validator();
};

#endif  // VALIDATOR_H
