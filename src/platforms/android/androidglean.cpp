/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidglean.h"
#include "androidauthenticationlistener.h"
#include "androidjnicompat.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "jni.h"
#include "mozillavpn.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookieJar>
#include <QUrlQuery>

#if QT_VERSION < 0x060000
#  include <QtAndroid>
#  include <QAndroidIntent>
#endif

namespace {
AndroidGlean* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidGlean");

constexpr auto UTILS_CLASS = "org/mozilla/firefox/vpn/qt/GleanUtil";
}  // namespace

// static
void AndroidGlean::initialize() {
  if (!s_instance) {
    Q_ASSERT(qApp);
    s_instance = new AndroidGlean(qApp);
  }
}

AndroidGlean::AndroidGlean(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AndroidGlean);
  Q_ASSERT(!s_instance);
  logger.debug() << "Connect Glean stuff";
  auto vpn = MozillaVPN::instance();
  connect(vpn, &MozillaVPN::initializeGlean, this,
          &AndroidGlean::initializeGlean);
  connect(vpn, &MozillaVPN::sendGleanPings, this,
          &AndroidGlean::sendGleanPings);
  connect(vpn, &MozillaVPN::recordGleanEvent, this,
          &AndroidGlean::recordGleanEvent);
  connect(vpn, &MozillaVPN::recordGleanEventWithExtraKeys, this,
          &AndroidGlean::recordGleanEventWithExtraKeys);
  connect(vpn, &MozillaVPN::setGleanSourceTags, this,
          &AndroidGlean::setGleanSourceTags);
  connect(SettingsHolder::instance(), &SettingsHolder::gleanEnabledChanged,
          this, &AndroidGlean::gleanEnabledChanged);
}

AndroidGlean::~AndroidGlean() {
  MVPN_COUNT_DTOR(AndroidGlean);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AndroidGlean::initializeGlean() {
  logger.debug() << "init GLEAN";
  bool upload = SettingsHolder::instance()->gleanEnabled();
  QString mode = Constants::inProduction() ? "production" : "staging";
  auto jMode = QJniObject::fromString(mode).object<jstring>();
  QJniObject::callStaticMethod<void>(UTILS_CLASS, "initializeGlean",
                                     "(ZLjava/lang/String;)V", upload, jMode);

}
void AndroidGlean::sendGleanPings() {
    logger.debug() << "Send glean pings";
  QJniObject::callStaticMethod<void>(UTILS_CLASS, "sendGleanPings");
}

void AndroidGlean::recordGleanEvent(const QString& gleanSampleName) {
  logger.debug() << " recordGleanEvent" << gleanSampleName;
  jstring jSampleName = QJniObject::fromString(gleanSampleName).object<jstring>();
  QJniObject::callStaticMethod<void>(UTILS_CLASS, "recordGleanEvent",
                                     "(Ljava/lang/String;)V", jSampleName);

}

void AndroidGlean::recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                                 const QVariantMap& extraKeys) {
  Q_UNUSED(extraKeys)
  Q_UNUSED(gleanSampleName)
  // TODO: Implement this
  return;
}
void AndroidGlean::setGleanSourceTags(const QStringList& tags) {
  // QString list = tags.join(",");
  // auto jList = QJniObject::fromString(list).object()
  // QJniObject::callStaticMethod<void>(UTILS_CLASS, "setGleanSourceTag",
  // "(ZLjava/lang/String;)",jList); return;
  Q_UNUSED(tags)
}

void AndroidGlean::gleanEnabledChanged(bool enabled) {
  QJniObject::callStaticMethod<void>(UTILS_CLASS, "setGleanUploadEnabled",
                                     "(Z)V", enabled);
}
