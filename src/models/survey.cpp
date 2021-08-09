/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "survey.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

namespace {
Logger logger(LOG_MAIN, "Survey");
}

Survey::Survey() { MVPN_COUNT_CTOR(Survey); }

Survey::Survey(const Survey& other) {
  MVPN_COUNT_CTOR(Survey);
  *this = other;
}

Survey& Survey::operator=(const Survey& other) {
  if (this == &other) return *this;

  m_id = other.m_id;
  m_url = other.m_url;
  m_triggerTime = other.m_triggerTime;
  m_platforms = other.m_platforms;

  return *this;
}

Survey::~Survey() { MVPN_COUNT_DTOR(Survey); }

bool Survey::fromJson(const QJsonValue& json) {
  if (!json.isObject()) {
    return false;
  }

  QJsonObject obj = json.toObject();

  QJsonValue id = obj.value("id");
  if (!id.isString()) {
    return false;
  }

  QJsonValue url = obj.value("url");
  if (!url.isString()) {
    return false;
  }

  QJsonValue triggerTime = obj.value("trigger_time");
  if (!triggerTime.isDouble()) {
    return false;
  }

  m_platforms.clear();
  if (obj.contains("platforms")) {
    QJsonValue platformArray = obj.value("platforms");
    if (!platformArray.isArray()) {
      return false;
    }
    for (auto val : platformArray.toArray()) {
      if (!val.isString()) {
        return false;
      }
      m_platforms.append(val.toString());
    }
  }

  m_id = id.toString();
  m_url = url.toString();
  m_triggerTime = triggerTime.toInt();

  return true;
}

bool Survey::isTriggerable() const {
  logger.debug() << "is triggerable";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->hasConsumedSurveys() &&
      settingsHolder->consumedSurveys().contains(m_id)) {
    return false;
  }

  if (!m_platforms.isEmpty() &&
      !m_platforms.contains(Constants::PLATFORM_NAME)) {
    return false;
  }

  QDateTime now = QDateTime::currentDateTime();
  QDateTime installation = settingsHolder->installationTime();

  // In minutes
  return (installation.secsTo(now) / 60) >= m_triggerTime;
}
