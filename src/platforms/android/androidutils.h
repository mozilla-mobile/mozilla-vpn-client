/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDUTILS_H
#define ANDROIDUTILS_H

#include <jni.h>
#include <QObject>
#include <QString>
#include <QUrl>

class AuthenticationListener;

class AndroidUtils final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidUtils)

  Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

 public:
  static QString GetDeviceName();

  static AndroidUtils* instance();

  void startAuthentication(AuthenticationListener* listener, const QUrl& url);

  const QUrl& url() const { return m_url; }

  Q_INVOKABLE void abortAuthentication();

  Q_INVOKABLE bool maybeCompleteAuthentication(const QString& url);

  Q_INVOKABLE void openNotificationSettings();

  static void dispatchToMainThread(std::function<void()> callback);

  static QByteArray getQByteArrayFromJString(JNIEnv* env, jstring data);

  static QString getQStringFromJString(JNIEnv* env, jstring data);

  static QJsonObject getQJsonObjectFromJString(JNIEnv* env, jstring data);

 signals:
  void urlChanged();

 private:
  AndroidUtils(QObject* parent);
  ~AndroidUtils();

  void resetListener() { m_listener = nullptr; }

 private:
  QUrl m_url;
  AuthenticationListener* m_listener = nullptr;
};

#endif  // ANDROIDUTILS_H
