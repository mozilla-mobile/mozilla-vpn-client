/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOCATION_H
#define LOCATION_H

#include <QHostAddress>
#include <QObject>
#include <QString>

class QJsonObject;
class QHostAddress;

class Location final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Location)

  Q_PROPERTY(QString cityName READ cityName NOTIFY changed)
  Q_PROPERTY(QString countryCode READ countryCode NOTIFY changed)
  Q_PROPERTY(QString subdivision READ subdivision NOTIFY changed)
  Q_PROPERTY(QHostAddress ipAddress READ ipAddress NOTIFY changed)
  Q_PROPERTY(double latitude READ latitude NOTIFY changed)
  Q_PROPERTY(double longitude READ longitude NOTIFY changed)

 public:
  Location();
  ~Location();

  [[nodiscard]] bool fromJson(const QByteArray& json);

  bool initialized() const { return m_initialized; }

  const QString& cityName() const { return m_cityName; }

  const QString& countryCode() const { return m_countryCode; }

  const QString& subdivision() const { return m_subdivision; }

  double latitude() const { return m_latitude; }

  double longitude() const { return m_longitude; }

  double distance(double latitude, double longitude) const;

  QHostAddress ipAddress() const { return m_ipAddress; }

 signals:
  void changed();

 private:
  QString m_cityName;
  QString m_countryCode;
  QString m_subdivision;
  QHostAddress m_ipAddress;
  double m_latitude;
  double m_longitude;
  double m_latSin;
  double m_latCos;
  bool m_initialized = false;
};

#endif  // LOCATION_H
