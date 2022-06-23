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
#include <QQmlEngine>
#include <QFileInfo>
#include <QFile>

namespace {
AndroidGlean* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidGlean");

}  // namespace

// static
void AndroidGlean::initialize(QQmlEngine* engine) {
  if (!s_instance) {
    Q_ASSERT(qApp);
    s_instance = new AndroidGlean(qApp);
  }
  // We might encounter a Glean Database from
  // the glean.js instance. As there is no use for it
  // we should remove it :)
  auto glean_db_path =
      engine->offlineStorageDatabaseFilePath("Glean") + ".sqlite";
  auto gleanDB = QFileInfo(glean_db_path);
  if (gleanDB.exists()) {
    QFile::remove(glean_db_path);
    logger.debug() << "Removed Glean.js DB";
  }
}

AndroidGlean::AndroidGlean(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AndroidGlean);
  Q_ASSERT(!s_instance);
  logger.debug() << "Connect Glean stuff";
  auto vpn = MozillaVPN::instance();
  connect(vpn, &MozillaVPN::sendGleanPings, this,
          &AndroidGlean::sendGleanMainPings);
  connect(vpn, &MozillaVPN::recordGleanEvent, this,
          &AndroidGlean::recordGleanEvent);
  connect(vpn, &MozillaVPN::recordGleanEventWithExtraKeys, this,
          &AndroidGlean::recordGleanEventWithExtraKeys);
  connect(vpn, &MozillaVPN::setGleanSourceTags, this,
          &AndroidGlean::setGleanSourceTags);
  connect(SettingsHolder::instance(), &SettingsHolder::gleanEnabledChanged,
          this, &AndroidGlean::gleanUploadEnabledChanged);
  connect(AndroidVPNActivity::instance(), &AndroidVPNActivity::serviceConnected,
          this, &AndroidGlean::daemonConnected);

  connect(qApp, &QApplication::applicationStateChanged, this,
          &AndroidGlean::applicationStateChanged);
  // Make sure we start the service now, so
  // glean is ready even before login.
  AndroidVPNActivity::connectService();
}

AndroidGlean::~AndroidGlean() {
  MVPN_COUNT_DTOR(AndroidGlean);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AndroidGlean::sendGleanMainPings() {
  AndroidVPNActivity::instance()->sendToService(
      ServiceAction::ACTION_SEND_GLEAN_MAIN_PING, "");
}

void AndroidGlean::recordGleanEvent(const QString& gleanSampleName) {
  QJsonObject args;
  args["key"] = gleanSampleName;
  QJsonDocument doc(args);
  logger.debug() << " recordGleanEvent" << gleanSampleName;
  AndroidVPNActivity::instance()->sendToService(
      ServiceAction::ACTION_RECORD_EVENT, doc.toJson(QJsonDocument::Compact));
}

void AndroidGlean::recordGleanEventWithExtraKeys(const QString& gleanSampleName,
                                                 const QVariantMap& extraKeys) {
  QJsonObject extras = QJsonObject::fromVariantMap(extraKeys);
  QJsonObject args;
  args["extras"] = extras;
  args["key"] = gleanSampleName;
  QJsonDocument doc(args);
  logger.debug() << " recordGleanEvent" << gleanSampleName;
  AndroidVPNActivity::instance()->sendToService(
      ServiceAction::ACTION_RECORD_EVENT, doc.toJson(QJsonDocument::Compact));
}
void AndroidGlean::setGleanSourceTags(const QStringList& tags) {
  AndroidVPNActivity::instance()->sendToService(
      ServiceAction::ACTION_GLEAN_SET_SOURCE_TAGS, tags.join(","));
}

void AndroidGlean::gleanUploadEnabledChanged(bool enabled) {
  QJsonObject args;
  args["enabled"] = enabled;
  QJsonDocument doc(args);
  logger.debug() << " gleanEnabledChanged" << enabled;
  AndroidVPNActivity::instance()->sendToService(
      ServiceAction::ACTION_GLEAN_ENABLED_CHANGED,
      doc.toJson(QJsonDocument::Compact));
}

void AndroidGlean::daemonConnected() {
  // Daemon is now ready
  gleanUploadEnabledChanged(SettingsHolder::instance()->gleanEnabled());
}

void AndroidGlean::applicationStateChanged(Qt::ApplicationState state) {
  switch (state) {
    case Qt::ApplicationState::ApplicationActive:
      break;
    case Qt::ApplicationState::ApplicationSuspended:
      [[fallthrough]];
    case Qt::ApplicationState::ApplicationInactive:
      [[fallthrough]];
    case Qt::ApplicationState::ApplicationHidden:
      logger.debug()
          << "App Going in the Background, trigger sending due pings";
      sendGleanMainPings();
      break;
  }
}