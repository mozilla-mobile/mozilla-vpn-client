/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCOMMONS_H
#define ANDROIDCOMMONS_H

#include <QJniObject>
#include <QObject>

class AndroidCommons final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidCommons)

 public:
  static QJniObject getActivity();

  static int getSDKVersion();

  static QString GetManufacturer();

  static bool verifySignature(const QByteArray& publicKey,
                              const QByteArray& content,
                              const QByteArray& signature);

  // Creates a "share" intent to Open/Send Plaintext
  static bool shareText(const QString& plainText);

  static void initializeGlean(bool isTelemetryEnabled, const QString& channel);

  static void runOnAndroidThreadSync(const std::function<void()> runnable);

  static void dispatchToMainThread(std::function<void()> callback);
};

#endif  // ANDROIDCOMMONS_H
