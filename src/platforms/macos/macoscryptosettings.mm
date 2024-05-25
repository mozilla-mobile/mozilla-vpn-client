/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscryptosettings.h"

#include "constants.h"
#include "cryptosettings.h"
#include "logger.h"

#import <Foundation/Foundation.h>

namespace {
Logger logger("MacOSCryptoSettings");
}  // namespace

MacOSCryptoSettings::MacOSCryptoSettings() : CryptoSettings() {
  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (appId) {
    m_appId QString::fromNSString(appId);
  } else {
#ifdef MZ_IOS
    m_appId = Constants::IOS_FALLBACK_APP_ID;
#else
    m_appId = Constants::MACOS_FALLBACK_APP_ID;
#endif
  }
}

void MacOSCryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";

  NSData* service = QByteArray(Constants::CRYPTO_SETTINGS_SERVICE).toNSData();
  NSString* appId = m_appId.toNSString();

  NSMutableDictionary* query = [[NSMutableDictionary alloc] init];

  [query setObject:(id)kSecClassGenericPassword forKey:(id)kSecClass];
  [query setObject:service forKey:(id)kSecAttrGeneric];
  [query setObject:service forKey:(id)kSecAttrAccount];
  [query setObject:appId forKey:(id)kSecAttrService];

  SecItemDelete((CFDictionaryRef)query);

  [query release];

  m_initialized = false;
}

QByteArray MacOSCryptoSettings::getKey() {
  if (!m_initialized) {
    m_initialized = true;

    logger.debug() << "Retrieving the key from the keychain";

    NSData* service = QByteArray(Constants::CRYPTO_SETTINGS_SERVICE).toNSData();
    NSString* appId = m_appId.toNSString();

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
      m_key = QByteArray::fromNSData(keyData);
      logger.debug() << "Key found with length:" << m_key.length();
      return m_key;
    }

    logger.warning() << "Key not found. Let's create it. Error:" << status;
    m_key = generateRandomBytes(CRYPTO_SETTINGS_KEY_SIZE);

    query = [[NSMutableDictionary alloc] init];

    [query setObject:(id)kSecClassGenericPassword forKey:(id)kSecClass];
    [query setObject:service forKey:(id)kSecAttrGeneric];
    [query setObject:service forKey:(id)kSecAttrAccount];
    [query setObject:appId forKey:(id)kSecAttrService];

    SecItemDelete((CFDictionaryRef)query);

    keyData = m_key.toNSData();
    [query setObject:keyData forKey:(id)kSecValueData];

    status = SecItemAdd((CFDictionaryRef)query, NULL);

    if (status != noErr) {
      logger.error() << "Failed to store the key. Error:" << status;
      m_key = QByteArray();
    }

    [query release];
  }

  return m_key;
}

CryptoSettings::Version MacOSCryptoSettings::getSupportedVersion() {
  logger.debug() << "Get supported settings method";

  if (getKey().isEmpty()) {
    logger.debug() << "No encryption";
    return CryptoSettings::NoEncryption;
  }
  logger.debug() << "Encryption supported!";
  return CryptoSettings::EncryptionChachaPolyV1;
}
