/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCOMMONS_H
#define ANDROIDCOMMONS_H

#include <QObject>

class AndroidCommons final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidCommons)

 public:
  static bool verifySignature(const QByteArray& publicKey,
                              const QByteArray& content,
                              const QByteArray& signature);
};

#endif  // ANDROIDCOMMONS_H
