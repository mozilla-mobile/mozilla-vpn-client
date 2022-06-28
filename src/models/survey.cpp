/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "survey.h"
#include "constants.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

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
  m_triggerTimeSec = other.m_triggerTimeSec;
  m_platforms = other.m_platforms;
  m_locales = other.m_locales;

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

  m_locales.clear();
  if (obj.contains("locales")) {
    QJsonValue localeArray = obj.value("locales");
    if (!localeArray.isArray()) {
      return false;
    }
    for (auto val : localeArray.toArray()) {
      if (!val.isString()) {
        return false;
      }
      m_locales.append(val.toString().toLower());
    }
  }

  m_id = id.toString();
  m_url = url.toString();
  m_triggerTimeSec = triggerTime.toInt();

  return true;
}

bool Survey::isTriggerable() const {
  logger.debug() << "Survey check for" << m_id;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->consumedSurveys().contains(m_id)) {
    logger.debug() << "Survey already consumed";
    return false;
  }

  if (!m_platforms.isEmpty() &&
      !m_platforms.contains(Constants::PLATFORM_NAME)) {
    logger.debug() << "is not right Platform";
    return false;
  }

  if (!m_locales.isEmpty()) {
    QString code = settingsHolder->languageCode();
    if (code.isEmpty()) {
      code = QLocale::system().bcp47Name();
      if (code.isEmpty()) {
        code = "en";
      }
    }

    code = Localizer::majorLanguageCode(code);

    if (!m_locales.contains(code.toLower())) {
      logger.debug() << "Locale does not match:" << code << m_locales;
      return false;
    }
  }

  QDateTime now = QDateTime::currentDateTime();
  QDateTime installation = settingsHolder->installationTime();

  // Note: m_triggerTimeSec is seconds!
  bool ok = (installation.secsTo(now)) >= m_triggerTimeSec;
  if (!ok) {
    logger.debug() << "This survey will be shown in: "
                   << m_triggerTimeSec - installation.secsTo(now) << "s";
  }
  return ok;
}

// static
QString Survey::replaceUrlParams(const QString& input) {
  QUrl url(input);

  if (!url.isValid()) {
    logger.error() << "Invalid survey URL:" << input;
    return input;
  }

  QUrlQuery currentQuery(url.query());
  QUrlQuery newQuery;

  for (QPair<QString, QString>& item : currentQuery.queryItems()) {
    if (item.second == "__VPN_VERSION__") {
      newQuery.addQueryItem(item.first, MozillaVPN::versionString());
    } else if (item.second == "__VPN_BUILDNUMBER__") {
      newQuery.addQueryItem(item.first, MozillaVPN::buildNumber());
    } else if (item.second == "__VPN_OS__") {
      newQuery.addQueryItem(item.first, MozillaVPN::osVersion());
    } else if (item.second == "__VPN_PLATFORM__") {
      newQuery.addQueryItem(item.first, MozillaVPN::platform());
    } else if (item.second == "__VPN_ARCH__") {
      newQuery.addQueryItem(item.first, MozillaVPN::architecture());
    } else if (item.second == "__VPN_GRAPHICSAPI__") {
      newQuery.addQueryItem(item.first, MozillaVPN::graphicsApi());
    } else {
      newQuery.addQueryItem(item.first, item.second);
    }
  }

  url.setQuery(newQuery);
  return url.toString();
  ;
}
