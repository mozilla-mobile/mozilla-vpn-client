/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidutils.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "platforms/android/androidauthenticationlistener.h"
#include "qmlengineholder.h"

#include <jni.h>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QApplication>
#include <QNetworkCookieJar>
#include <QUrlQuery>
#include <QtAndroid>

namespace {
AndroidUtils* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidUtils");
}  // namespace

// static
QString AndroidUtils::GetDeviceName() {
  QAndroidJniEnvironment env;
  jclass BUILD = env->FindClass("android/os/Build");
  jfieldID model = env->GetStaticFieldID(BUILD, "MODEL", "Ljava/lang/String;");
  jstring value = (jstring)env->GetStaticObjectField(BUILD, model);
  if (!value) {
    return QString("Android Device");
  }
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    return QString("Android Device");
  }
  QString res = QString(buffer);
  env->ReleaseStringUTFChars(value, buffer);
  return res;
};

// static
AndroidUtils* AndroidUtils::instance() {
  if (!s_instance) {
    Q_ASSERT(qApp);
    s_instance = new AndroidUtils(qApp);
  }

  return s_instance;
}

AndroidUtils::AndroidUtils(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AndroidUtils);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

AndroidUtils::~AndroidUtils() {
  MVPN_COUNT_DTOR(AndroidUtils);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AndroidUtils::startAuthentication(AuthenticationListener* listener,
                                       const QUrl& url) {
  logger.debug() << "Open the authentication view";

  Q_ASSERT(!m_listener);
  m_listener = listener;

  connect(listener, &QObject::destroyed, this, &AndroidUtils::resetListener);

  m_url = url;
  emit urlChanged();

  emit MozillaVPN::instance()->loadAndroidAuthenticationView();
}

bool AndroidUtils::maybeCompleteAuthentication(const QString& url) {
  logger.debug() << "Maybe complete authentication - url:" << url;

  Q_ASSERT(m_listener);

  logger.debug() << "AndroidWebView is about to load" << url;

  QString apiUrl = NetworkRequest::apiBaseUrl();
  if (!url.startsWith(apiUrl)) {
    return false;
  }

  QUrl loadingUrl(url);
  if (loadingUrl.path() == "/vpn/client/login/success") {
    QUrlQuery query(loadingUrl.query());
    if (!query.hasQueryItem("code")) {
      emit m_listener->failed(ErrorHandler::RemoteServiceError);
      m_listener = nullptr;
      return true;
    }

    QString code = query.queryItemValue("code");
    emit m_listener->completed(code);
    m_listener = nullptr;
    return true;
  }

  if (loadingUrl.path() == "/vpn/client/login/error") {
    emit m_listener->failed(ErrorHandler::AuthenticationError);
    m_listener = nullptr;
    return true;
  }

  return false;
}

void AndroidUtils::abortAuthentication() {
  logger.warning() << "Aborting authentication";

  Q_ASSERT(m_listener);
  emit m_listener->abortedByUser();
  m_listener = nullptr;
}

// static
void AndroidUtils::appReviewRequested() {
  QAndroidJniObject activity = QtAndroid::androidActivity();
  Q_ASSERT(activity.isValid());

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/qt/VPNAppReview", "appReviewRequested",
      "(Landroid/app/Activity;)V", activity.object<jobject>());
}
