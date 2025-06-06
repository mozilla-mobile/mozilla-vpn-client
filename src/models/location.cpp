/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "location.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("Location");
}  // namespace

Location::Location() {
  MZ_COUNT_CTOR(Location);

  m_latitude = qQNaN();
  m_longitude = qQNaN();
  m_latSin = qQNaN();
  m_latCos = qQNaN();
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
    m_latSin = qSin(m_latitude * M_PI / 180.0);
    m_latCos = qCos(m_latitude * M_PI / 180.0);
  }
  if (!lat_okay || !long_okay) {
    m_latitude = qQNaN();
    m_longitude = qQNaN();
    m_latSin = qQNaN();
    m_latCos = qQNaN();
  }

  m_cityName = cityName.toString();
  m_countryCode = countryCode.toString();
  m_subdivision = subdivision.toString();
  m_ipAddress = ipAddress;
  m_initialized = true;

  logger.debug() << "Location calculated as" << logger.sensitive(m_cityName)
                 << "in" << m_countryCode << "-"
                 << logger.sensitive(m_subdivision);

  emit changed();
  return true;
}

// Compute distance between two points on a great circle, which is given by:
//   d = acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(long1-long2))
//
// This is done in spherical coordinates, and will return a value in the range
// of zero to pi. Multiply by the Earth's radius if you want meaningful units.
double Location::distance(double latitude, double longitude) const {
  if (qIsNaN(latitude) || qIsNaN(longitude)) {
    return 0.0;
  }
  if (qIsNaN(m_latitude) || qIsNaN(m_longitude)) {
    return 0.0;
  }
  Q_ASSERT(!qIsNaN(m_latSin));
  Q_ASSERT(!qIsNaN(m_latCos));
  double otherSin = qSin(latitude * M_PI / 180.0);
  double otherCos = qCos(latitude * M_PI / 180.0);
  double diffCos = qCos((longitude - m_longitude) * M_PI / 180.0);

  return qAcos(m_latSin * otherSin + m_latCos * otherCos * diffCos);
}

// The same algorithm as above, but static and accepts any QObject with
// latitude and longitude properties.
double Location::distance(const QObject* a, const QObject* b) {
  bool aLatOkay = false;
  bool aLongOkay = false;
  bool bLatOkay = false;
  bool bLongOkay = false;
  double aLat = a->property("latitude").toDouble(&aLatOkay);
  double aLong = a->property("longitude").toDouble(&aLongOkay);
  double bLat = b->property("latitude").toDouble(&bLatOkay);
  double bLong = b->property("longitude").toDouble(&bLongOkay);
  if (!aLatOkay || !aLongOkay || !bLatOkay || !bLongOkay) {
    return 0.0;
  }
  if (qIsNaN(aLat) || qIsNaN(aLong) || qIsNaN(bLat) || qIsNaN(bLong)) {
    return 0.0;
  }

  double aSin = qSin(aLat * M_PI / 180.0);
  double aCos = qCos(aLat * M_PI / 180.0);
  double bSin = qSin(bLat * M_PI / 180.0);
  double bCos = qCos(bLat * M_PI / 180.0);
  double diffCos = qCos((aLong - bLong) * M_PI / 180.0);

  return qAcos(aSin * bSin + aCos * bCos * diffCos);
}
