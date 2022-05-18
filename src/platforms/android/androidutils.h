/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDUTILS_H
#define ANDROIDUTILS_H

#include <jni.h>
#include <QObject>
#include <QString>
#include <QUrl>

#include <QJniObject>
#include <QJniEnvironment>

class AuthenticationListener;

class AndroidUtils final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidUtils)

 public:
  static QString GetDeviceName();

  static int GetSDKVersion();

  // Creates a "share" intent to Open/Send Plaintext
  static bool ShareText(const QString& plainText);

  static QByteArray DeviceId();

  static AndroidUtils* instance();

  Q_INVOKABLE void openNotificationSettings();

  static void dispatchToMainThread(std::function<void()> callback);

  static QByteArray getQByteArrayFromJString(JNIEnv* env, jstring data);

  static QString getQStringFromJString(JNIEnv* env, jstring data);

  static QJsonObject getQJsonObjectFromJString(JNIEnv* env, jstring data);

  static QJniObject getActivity();

  static void recordGleanEvent(JNIEnv* env, jobject VPNUtils, jstring event);

  static void runOnAndroidThreadSync(const std::function<void()> runnable);

 private:
  AndroidUtils(QObject* parent);
  ~AndroidUtils();
};

#endif  // ANDROIDUTILS_H
