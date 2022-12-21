/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "appconstants.h"
#include "cryptosettings.h"
#include "logger.h"

#include <QRandomGenerator>

#import <Foundation/Foundation.h>

namespace {

Logger logger("MacOSCryptoSettings");

bool initialized = false;
QByteArray key;

NSString* getAppId() {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (!appId) {
#ifdef MZ_IOS
    appId = QString(AppConstants::IOS_FALLBACK_APP_ID).toNSString();
#else
    appId = QString(AppConstants::MACOS_FALLBACK_APP_ID).toNSString();
#endif
  }
  return appId;
}

}  // anonymous

// static
void CryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";

  NSData* service = QByteArray(AppConstants::CRYPTO_SETTINGS_SERVICE).toNSData();
  NSString* appId = getAppId();

  NSMutableDictionary* query = [[NSMutableDictionary alloc] init];

  [query setObject:(id)kSecClassGenericPassword forKey:(id)kSecClass];
  [query setObject:service forKey:(id)kSecAttrGeneric];
  [query setObject:service forKey:(id)kSecAttrAccount];
  [query setObject:appId forKey:(id)kSecAttrService];

  SecItemDelete((CFDictionaryRef)query);

  [query release];

  initialized = false;
}

// static
bool CryptoSettings::getKey(uint8_t output[CRYPTO_SETTINGS_KEY_SIZE]) {
#if defined(MZ_IOS) || defined(MZ_MACOS)
  if (!initialized) {
    initialized = true;

    logger.debug() << "Retrieving the key from the keychain";

    NSData* service = QByteArray(AppConstants::CRYPTO_SETTINGS_SERVICE).toNSData();
    NSString* appId = getAppId();

    NSMutableDictionary* query = [[NSMutableDictionary alloc] init];

    [query setObject:(id)kSecClassGenericPassword forKey:(id)kSecClass];
    [query setObject:service forKey:(id)kSecAttrGeneric];
    [query setObject:service forKey:(id)kSecAttrAccount];
    [query setObject:appId forKey:(id)kSecAttrService];

    [query setObject:(id)kCFBooleanTrue forKey:(id)kSecReturnData];
    [query setObject:(id)kSecMatchLimitOne forKey:(id)kSecMatchLimit];

    NSData* keyData = NULL;
    OSStatus status = SecItemCopyMatching((CFDictionaryRef)query, (CFTypeRef*)(void*)&keyData);
    [query release];

    if (status == noErr) {
      key = QByteArray::fromNSData(keyData);

      logger.debug() << "Key found with length:" << key.length();
      if (key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
        memcpy(output, key.data(), CRYPTO_SETTINGS_KEY_SIZE);
        return true;
      }
    }

    logger.warning() << "Key not found. Let's create it. Error:" << status;
    key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
    QRandomGenerator* rg = QRandomGenerator::system();
    for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
      key[i] = rg->generate() & 0xFF;
    }

    query = [[NSMutableDictionary alloc] init];

    [query setObject:(id)kSecClassGenericPassword forKey:(id)kSecClass];
    [query setObject:service forKey:(id)kSecAttrGeneric];
    [query setObject:service forKey:(id)kSecAttrAccount];
    [query setObject:appId forKey:(id)kSecAttrService];

    SecItemDelete((CFDictionaryRef)query);

    keyData = key.toNSData();
    [query setObject:keyData forKey:(id)kSecValueData];

    status = SecItemAdd((CFDictionaryRef)query, NULL);

    if (status != noErr) {
      logger.error() << "Failed to store the key. Error:" << status;
      key = QByteArray();
    }

    [query release];
  }

  if (key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(output, key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }

  logger.error() << "Invalid key";
#else
  Q_UNUSED(output);
#endif

  return false;
}

// static
CryptoSettings::Version CryptoSettings::getSupportedVersion() {
  logger.debug() << "Get supported settings method";

#if defined(MZ_IOS) || defined(MZ_MACOS)
  uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
  if (getKey(key)) {
    logger.debug() << "Encryption supported!";
    return CryptoSettings::EncryptionChachaPolyV1;
  }
#endif

  logger.debug() << "No encryption";
  return CryptoSettings::NoEncryption;
}
