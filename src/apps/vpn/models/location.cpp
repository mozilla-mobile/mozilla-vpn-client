/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "location.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "leakdetector.h"

Location::Location() {
  MZ_COUNT_CTOR(Location);

  m_latitude = qQNaN();
  m_longitude = qQNaN();
}

Location::~Location() { MZ_COUNT_DTOR(Location); }

bool Location::fromJson(const QByteArray& json) {
  m_initialized = false;

  QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  QJsonValue cityName = obj.value("city");
  if (!cityName.isString()) {
    return false;
  }

  QJsonValue countryCode = obj.value("country");
  if (!countryCode.isString()) {
    return false;
  }

  QJsonValue subdivision = obj.value("subdivision");
  if (!subdivision.isString()) {
    return false;
  }

  QJsonValue ipString = obj.value("ip");
  if (!ipString.isString()) {
    return false;
  }
  QHostAddress ipAddress(ipString.toString());
  if (ipAddress.isNull()) {
    return false;
  }

  // Added in Guardian 1.27.3 - it may not exist on prod yet.
  QStringList latlong = obj.value("lat_long").toString().split(',');
  bool lat_okay = false;
  bool long_okay = false;
  if (latlong.count() >= 2) {
    m_latitude = latlong[0].toDouble(&lat_okay);
    m_longitude = latlong[1].toDouble(&long_okay);
  }
  if (!lat_okay || !long_okay) {
    m_latitude = qQNaN();
    m_longitude = qQNaN();
  }

  m_cityName = cityName.toString();
  m_countryCode = countryCode.toString();
  m_subdivision = subdivision.toString();
  m_ipAddress = ipAddress;
  m_initialized = true;

  emit changed();
  return true;
}
