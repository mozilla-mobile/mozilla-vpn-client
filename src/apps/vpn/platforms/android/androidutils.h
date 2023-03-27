/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDUTILS_H
#define ANDROIDUTILS_H

#include <jni.h>

#include <QJniEnvironment>
#include <QJniObject>
#include <QObject>
#include <QString>
#include <QUrl>

class AuthenticationListener;

class AndroidUtils final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidUtils)

 public:
  static QString GetDeviceName();

  static QByteArray DeviceId();

  static AndroidUtils* instance();

  Q_INVOKABLE void openNotificationSettings();

  static QByteArray getQByteArrayFromJString(JNIEnv* env, jstring data);

  static QJsonObject getQJsonObjectFromJString(JNIEnv* env, jstring data);

 private:
  AndroidUtils(QObject* parent);
  ~AndroidUtils();
};

#endif  // ANDROIDUTILS_H
