/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QtTest/QtTest>

class TestModels: public QObject
{
    Q_OBJECT

private slots:
    void deviceBasic();
    void deviceFromJson_data();
    void deviceFromJson();

    void deviceModelBasic();
    void deviceModelFromJson_data();
    void deviceModelFromJson();

    void keysBasic();

    // TODO: servercity
    // TODO: servercountry

    void serverDataBasic();

    void userBasic();
    void userFromJson_data();
    void userFromJson();
};
