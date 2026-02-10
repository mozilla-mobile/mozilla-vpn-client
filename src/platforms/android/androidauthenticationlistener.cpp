/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidauthenticationlistener.h"

#include <QJniEnvironment>
#include <QJniObject>
#include <QUrlQuery>

#include "androidvpnactivity.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/android/androidcommons.h"
#include "urlopener.h"

namespace {
Logger logger("AndroidAuthenticationListener");

constexpr int CUSTOM_TAB_SUCCESS = 0;
constexpr int CUSTOM_TAB_ERROR_NO_CUSTOM_TABS = 1;
constexpr int CUSTOM_TAB_ERROR_INVALID_URL = 2;
constexpr int CUSTOM_TAB_ERROR_LAUNCH_FAILED = 3;
}  // namespace

AndroidAuthenticationListener::AndroidAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MZ_COUNT_CTOR(AndroidAuthenticationListener);
}

AndroidAuthenticationListener::~AndroidAuthenticationListener() {
  MZ_COUNT_DTOR(AndroidAuthenticationListener);
}

void AndroidAuthenticationListener::start(Task* task,
                                          const QString& codeChallenge,
                                          const QString& codeChallengeMethod,
                                          const QString& emailAddress) {
  logger.debug() << "AndroidAuthenticationListener initialize";

  Q_UNUSED(task);

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod,
                                   emailAddress));

  QUrlQuery query(url.query());
  query.addQueryItem("utm_medium", "vpn-client");
  query.addQueryItem("utm_source", "android-customtab-flow");
  url.setQuery(query);

  QString urlString = url.toString();
  logger.debug() << "Opening authentication URL";

  QJniObject activity = AndroidCommons::getActivity();
  QJniObject jniUrl = QJniObject::fromString(urlString);

  jint result = QJniObject::callStaticMethod<jint>(
      CUSTOM_TAB_HELPER_CLASS, "openCustomTab",
      "(Landroid/content/Context;Ljava/lang/String;)I", activity.object(),
      jniUrl.object<jstring>());

  if (AndroidCommons::clearPendingJavaException("openCustomTab")) {
    result = CUSTOM_TAB_ERROR_LAUNCH_FAILED;
  }

  switch (result) {
    case CUSTOM_TAB_SUCCESS:
      logger.debug() << "Custom Tab launched successfully";
      connect(AndroidVPNActivity::instance(),
              &AndroidVPNActivity::customTabClosed, this,
              &AndroidAuthenticationListener::abortedByUser);
      break;

    case CUSTOM_TAB_ERROR_NO_CUSTOM_TABS:
      logger.warning()
          << "Custom Tabs not available, falling back to system browser";
      UrlOpener::instance()->openUrl(url);
      break;

    case CUSTOM_TAB_ERROR_INVALID_URL:
      logger.error() << "Invalid URL for Custom Tab";
      emit failed(ErrorHandler::AuthenticationError);
      return;

    case CUSTOM_TAB_ERROR_LAUNCH_FAILED:
      logger.warning()
          << "Custom Tab launch failed, falling back to system browser";
      UrlOpener::instance()->openUrl(url);
      break;

    default:
      logger.warning() << "Unknown Custom Tab result:" << result
                       << ", falling back to system browser";
      UrlOpener::instance()->openUrl(url);
      break;
  }
}
