/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscryptosettings.h"

#include "constants.h"
#include "cryptosettings.h"
#include "logger.h"

#import <Foundation/Foundation.h>
#ifdef MZ_MACOS
#  import <Security/SecTask.h>
#endif

namespace {
Logger logger("MacOSCryptoSettings");
}  // namespace

MacOSCryptoSettings::MacOSCryptoSettings() : CryptoSettings() {
  if (checkCodesign() && checkEntitlement("keychain-access-groups")) {
    // If we are signed and have entitlements - we can use the encryption key.
    m_keyVersion = CryptoSettings::EncryptionChachaPolyV1;
  } else {
    logger.warning() << "Disabling encryption: Codesign is invalid";
    m_keyVersion = CryptoSettings::NoEncryption;
  }

  NSString* appId = [[NSBundle mainBundle] bundleIdentifier];
  if (appId) {
    m_appId = QString::fromNSString(appId);
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

QByteArray MacOSCryptoSettings::getKey(const QByteArray& metadata) {
  Q_UNUSED(metadata);

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

// static
bool MacOSCryptoSettings::checkCodesign() {
#ifdef MZ_MACOS
  SecTaskRef task = SecTaskCreateFromSelf(kCFAllocatorSystemDefault);
  CFStringRef signer = SecTaskCopySigningIdentifier(task, nullptr);
  CFRelease(task);
  if (signer != nullptr) {
    logger.debug() << "Got signature from:"
                   << QString::fromNSString(static_cast<NSString*>(signer));
    CFRelease(signer);
    return true;
  }
  return false;
#else
  // For iOS we probably need to roll our own solution by calling the
  // csopt syscall directly, as the SecTask framework is only available
  // for MacOS.
  return true;
#endif
}

// static
bool MacOSCryptoSettings::checkEntitlement(const QString& name) {
#ifdef MZ_MACOS
  SecTaskRef task = SecTaskCreateFromSelf(kCFAllocatorSystemDefault);
  if (task == nullptr) {
    return false;
  }
  CFStringRef cfName = CFStringCreateWithCString(
      kCFAllocatorSystemDefault, qUtf8Printable(name), kCFStringEncodingUTF8);
  auto guard = qScopeGuard([&] {
    CFRelease(task);
    CFRelease(cfName);
  });

  CFErrorRef error = nullptr;
  CFTypeRef result = SecTaskCopyValueForEntitlement(task, cfName, &error);
  if (error != nullptr) {
    CFStringRef desc = CFErrorCopyDescription(error);
    logger.error() << "Failed to check entitlements:"
                   << QString::fromNSString(static_cast<NSString*>(desc));
    CFRelease(desc);
    CFRelease(error);
  }
  if (result != nullptr) {
    CFRelease(result);
  }

  // Return success if we got anything back from the entitlement.
  return (result != nullptr);
#else
  // For iOS we probably need to roll our own solution by calling the
  // csopt syscall directly, as the SecTask methods are only available
  // for MacOS.
  //
  // Check out https://github.com/Apple-FOSS-Mirror/security_systemkeychain/
  // for inspiration, specifically the procinfo() function in src/cs_misc.cpp.
  return true;
#endif
}
