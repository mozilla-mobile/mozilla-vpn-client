/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportal.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("CaptivePortal");
}

CaptivePortal::CaptivePortal() { MZ_COUNT_CTOR(CaptivePortal); }

CaptivePortal::~CaptivePortal() { MZ_COUNT_DTOR(CaptivePortal); }

bool CaptivePortal::fromJson(const QByteArray& data) {
  logger.debug() << "Captive portal from Json";

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isArray()) {
    logger.error() << "Invalid JSON array";
    return false;
  }

  QStringList ipv4Addresses;
  QStringList ipv6Addresses;

  QJsonArray array = json.array();
  for (const QJsonValue& entry : array) {
    if (!entry.isObject()) {
      logger.error() << "Object expected";
      return false;
    }

    QJsonObject obj = entry.toObject();
    QJsonValue address = obj.value("address");
    if (!address.isString()) {
      logger.error() << "Address must be a string";
      return false;
    }

    QJsonValue family = obj.value("family");
    if (!family.isDouble()) {
      logger.error() << "Family must be a number";
      return false;
    }

    switch (family.toInt()) {
      case 4:
        ipv4Addresses.append(address.toString());
        break;
      case 6:
        ipv6Addresses.append(address.toString());
        break;
      default:
        logger.error() << "Invalid family number" << family.toInt();
        return false;
    }
  }

  m_ipv4Addresses = ipv4Addresses;
  m_ipv6Addresses = ipv6Addresses;
  return true;
}

bool CaptivePortal::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  m_ipv4Addresses = settingsHolder->captivePortalIpv4Addresses();
  m_ipv6Addresses = settingsHolder->captivePortalIpv6Addresses();

  return true;
}

void CaptivePortal::writeSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setCaptivePortalIpv4Addresses(m_ipv4Addresses);
  settingsHolder->setCaptivePortalIpv6Addresses(m_ipv6Addresses);
}
