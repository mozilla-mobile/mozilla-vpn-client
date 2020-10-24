/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserverdata.h"
#include "../src/servercity.h"
#include "../src/servercountry.h"
#include "../src/serverdata.h"
#include "autotest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestServerData::basic()
{
    ServerData sd;

    QCOMPARE(sd.initialized(), false);
    QCOMPARE(sd.countryCode(), "");
    QCOMPARE(sd.city(), "");

    {
        QJsonObject countryObj;
        countryObj.insert("name", "serverCountryName");
        countryObj.insert("code", "serverCountryCode");
        countryObj.insert("cities", QJsonArray());
        ServerCountry country = ServerCountry::fromJson(countryObj);

        QJsonObject cityObj;
        cityObj.insert("code", "serverCityCode");
        cityObj.insert("name", "serverCityName");
        cityObj.insert("servers", QJsonArray());
        ServerCity city = ServerCity::fromJson(cityObj);

        sd.initialize(country, city);
        QCOMPARE(sd.initialized(), true);
        QCOMPARE(sd.countryCode(), "serverCountryCode");
        QCOMPARE(sd.city(), "serverCityName");
    }

    sd.update("new Country", "new City");
    QCOMPARE(sd.initialized(), true);
    QCOMPARE(sd.countryCode(), "new Country");
    QCOMPARE(sd.city(), "new City");

    sd.forget();
    QCOMPARE(sd.initialized(), false);
    QCOMPARE(sd.countryCode(), "new Country");
    QCOMPARE(sd.city(), "new City");
}

DECLARE_TEST(TestServerData)
