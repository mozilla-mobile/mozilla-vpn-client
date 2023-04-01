/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERI18N_H
#define SERVERI18N_H

#include <QHash>
#include <QObject>

class ServerI18N final : public QObject {
 public:
  static ServerI18N* instance();

  ~ServerI18N();

  QString translateCountryName(const QString& countryCode,
                               const QString& countryName);

  QString translateCityName(const QString& countryCode,
                            const QString& cityName);

 private:
  ServerI18N(QObject* parent);

  void initialize();

  QString translateItem(const QString& countryCode, const QString& cityName,
                        const QString& fallback);

  QString translateItemWithLanguage(const QString& languageCode,
                                    const QString& countryCode,
                                    const QString& cityName);

  void addCity(const QString& countryCode, const QJsonValue& value);
  void addCountry(const QJsonValue& value);

 private:
  QHash<QString, QString> m_items;
};

#endif  // SERVERI18N_H
