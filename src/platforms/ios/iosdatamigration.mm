/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosdatamigration.h"
#include "device.h"
#include "logger.h"
#include "mozillavpn.h"
#include "user.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#import <Foundation/Foundation.h>

namespace {
Logger logger(LOG_IOS, "IOSDataMigration");

void migrateUserDefaultData()
{
    MozillaVPN *vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
    if (!sud) {
        return;
    }

    NSData *userData = [sud dataForKey:@"user"];
    if (userData) {
        QByteArray json = QByteArray::fromNSData(userData);
        if (!json.isEmpty()) {
            logger.log() << "User data to be migrated";
            vpn->accountChecked(json);
        }
    }

    NSData *deviceData = [sud dataForKey:@"device"];
    if (deviceData) {
        QByteArray json = QByteArray::fromNSData(deviceData);
        logger.log() << "Device data to be migrated";
        // Nothing has to be done here because the device data is part of the user data.
    }

    NSData *serversData = [sud dataForKey:@"vpnServers"];
    if (serversData) {
        QByteArray json = QByteArray::fromNSData(serversData);
        if (!json.isEmpty()) {
            logger.log() << "Server list data to be migrated";

            // We need to wrap the server list in a object to make it similar to the REST API response.
            QJsonDocument serverList = QJsonDocument::fromJson(json);
            if (!serverList.isArray()) {
                logger.log() << "Server list should be an array!";
                return;
            }

            QJsonObject countriesObj;
            countriesObj.insert("countries", QJsonValue(serverList.array()));

            QJsonDocument doc;
            doc.setObject(countriesObj);
            vpn->setServerList(doc.toJson());
        }
    }

    NSData *selectedCityData = [sud dataForKey:@"selectedCity"];
    if (selectedCityData) {
        QByteArray json = QByteArray::fromNSData(selectedCityData);
        logger.log() << "SelectedCity data to be migrated" << json;
        // Nothing has to be done here because the device data is part of the user data.

        QJsonDocument doc = QJsonDocument::fromJson(json);
        if (!doc.isObject()) {
            logger.log() << "SelectedCity should be an object";
            return;
        }

        QJsonObject obj = doc.object();
        QJsonValue code = obj.take("flagCode");
        if (!code.isString()) {
            logger.log() << "SelectedCity code should be a string";
            return;
        }

        QJsonValue name = obj.take("code");
        if (!name.isString()) {
            logger.log() << "SelectedCity name should be a string";
            return;
        }

        ServerData serverData;
        if (vpn->serverCountryModel()->pickIfExists(code.toString(), name.toString(), serverData)) {
            logger.log() << "ServerCity found";
            serverData.writeSettings(*vpn->settingsHolder());
        }
    }
}

void migrateKeychainData()
{
    NSData *service = [@"org.mozilla.guardian.credentials" dataUsingEncoding:NSUTF8StringEncoding];

    NSMutableDictionary *query = [[NSMutableDictionary alloc] init];

    [query setObject:(id) kSecClassGenericPassword forKey:(id) kSecClass];
    [query setObject:service forKey:(id)kSecAttrService];
    [query setObject:(id) kCFBooleanTrue forKey:(id) kSecReturnData];
    [query setObject:(id) kSecMatchLimitOne forKey:(id) kSecMatchLimit];

    NSData *dataNS = NULL;
    OSStatus status = SecItemCopyMatching((CFDictionaryRef)query, (CFTypeRef*)(void*)&dataNS);
    [query release];

    if (status != noErr) {
        logger.log() << "No credentials found";
        return;
    }

    QByteArray data = QByteArray::fromNSData(dataNS);
    logger.log() << "Credentials:" << data;

    QJsonDocument json = QJsonDocument::fromJson(data);
    if (!json.isObject()) {
        logger.log() << "JSON object expected";
        return;
    }

    QJsonObject obj = json.object();
    QJsonValue deviceKeyValue = obj.take("deviceKeys");
    if (!deviceKeyValue.isObject()) {
        logger.log() << "JSON object should have a deviceKeys object";
        return;
    }

    QJsonObject deviceKeyObj = deviceKeyValue.toObject();
    QJsonValue publicKey = deviceKeyObj.take("publicKey");
    if (!publicKey.isString()) {
        logger.log() << "JSON deviceKey object should contain a publicKey value as string";
        return;
    }

    QJsonValue privateKey = deviceKeyObj.take("privateKey");
    if (!privateKey.isString()) {
        logger.log() << "JSON deviceKey object should contain a privateKey value as string";
        return;
    }

    QJsonValue token = obj.take("verificationToken");
    if (!token.isString()) {
        logger.log() << "JSON object should contain a verificationToken value s string";
        return;
    }

    MozillaVPN::instance()->deviceAdded(Device::currentDeviceName(),
                                        publicKey.toString(),
                                        privateKey.toString());

    MozillaVPN::instance()->setToken(token.toString());
}

}

// static
void IOSDataMigration::migrate()
{
    logger.log() << "IOS Data Migration";

    migrateKeychainData();
    migrateUserDefaultData();
}
