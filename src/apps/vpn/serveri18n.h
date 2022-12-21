/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERI18N_H
#define SERVERI18N_H

#include <QString>

class ServerI18N final {
 public:
  static QString translateCountryName(const QString& countryCode,
                                      const QString& countryName);

  static QString translateCityName(const QString& countryCode,
                                   const QString& cityName);
};

#endif  // SERVERI18N_H
