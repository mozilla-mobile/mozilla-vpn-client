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
  /**
   * @brief Returns the name of the Device.
   * Checks: Build.DEVICE
   * Unlike GetDeviceName, returns the internal model name.
   * I.e "Google Pixel 7" is 'Panther'
   * @return QString of Build.DEVICE
   */
  static QString getDeviceCodename();

  /**
   * @brief Get the Device Model Name
   * Checks: Build.Model
   * Name here means how the manufacturer calls it.
   * I.e "Google Pixel 7" is .. a Google Pixel 7
   * @return QString of Build.Model
   */
  static QString getDeviceName();

  /**
   * @brief Checks if we are running on ChromeOS+ Android Subsystem
   *
   */
  static bool isChromeOSContext();

  static QByteArray DeviceId();

  static AndroidUtils* instance();

  Q_INVOKABLE void openNotificationSettings();

  static QByteArray getQByteArrayFromJString(JNIEnv* env, jstring data);

  static QJsonObject getQJsonObjectFromJString(JNIEnv* env, jstring data);

 private:
  static QString readStaticString(const char* classname,
                                  const char* propertyName);

  AndroidUtils(QObject* parent);
  ~AndroidUtils();
};

#endif  // ANDROIDUTILS_H
