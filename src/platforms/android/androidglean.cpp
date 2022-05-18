/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidglean.h"
#include "androidutils.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "jni.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "androidvpnactivity.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJniObject>
#include <QJniEnvironment>
#include <QQmlApplicationEngine>
#include <QJSEngine>

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

  AndroidUtils::runOnAndroidThreadSync([this]() {
    logger.debug() << "Initialize Glean";
    initializeGlean();
  });
}

AndroidGlean::~AndroidGlean() {
  MVPN_COUNT_DTOR(AndroidGlean);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AndroidGlean::initializeGlean() {
  logger.debug() << "init GLEAN";
  QJsonDocument args;
  args['upload'] = SettingsHolder::instance()->gleanEnabled();;
  args['channel']= Constants::inProduction() ? "production" : "staging";
  AndroidVPNActivity::instance->sendToService(AndroidVPNActivity::ServiceAction::ACTION_GLEAN_INIT, args.toJson(QJsonDocument::Compact));
}
void AndroidGlean::sendGleanPings() {
  AndroidVPNActivity::instance->sendToService(AndroidVPNActivity::ServiceAction::ACTION_SEND_GLEAN_PING, "");
}

void AndroidGlean::recordGleanEvent(const QString& gleanSampleName) {
  QJsonDocument args;
  args['key']= gleanSampleName;
  logger.debug() << " recordGleanEvent" << gleanSampleName;
  AndroidVPNActivity::instance->sendToService(AndroidVPNActivity::ServiceAction::ACTION_RECORD_EVENT, args.toJson(QJsonDocument::Compact));
}

void AndroidGlean::recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                                 const QVariantMap& extraKeys) {
  QJsonObject extras = QJsonObject::fromVariantMap(extraKeys);
  QJsonDocument args;
  args['extras'] = extras;
  args['key']= gleanSampleName;
  logger.debug() << " recordGleanEvent" << gleanSampleName;
  AndroidVPNActivity::instance->sendToService(AndroidVPNActivity::ServiceAction::ACTION_RECORD_EVENT, args.toJson(QJsonDocument::Compact));

}
void AndroidGlean::setGleanSourceTags(const QStringList& tags) {
  // TODO: Current no-op as glean-android does not implement this - no need to
  // do anything QString list = tags.join(","); auto jList =
  // QJniObject::fromString(list).object()
  // QJniObject::callStaticMethod<void>(UTILS_CLASS, "setGleanSourceTag",
  // "(ZLjava/lang/String;)",jList); return;
  Q_UNUSED(tags)
}

void AndroidGlean::gleanEnabledChanged(bool enabled) {
  QJsonDocument args;
  args['enabled']=enabled;
  logger.debug() << " gleanEnabledChanged" << enabled;
  AndroidVPNActivity::instance->sendToService(AndroidVPNActivity::ServiceAction::ACTION_GLEAN_ENABLED_CHANGED, args.toJson(QJsonDocument::Compact));
}
