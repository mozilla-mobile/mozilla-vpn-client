/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cryptosettings.h"
#include "logger.h"

#include <QRandomGenerator>

#include <Windows.h>
#include <wincrypt.h>
#include <wincred.h>

#define CRED_KEY L"Mozilla VPN"

namespace {
Logger logger(LOG_WINDOWS, "CryptoSettings");

bool s_initialized = false;
QByteArray s_key;
}  // namespace

void CryptoSettings::resetKey() {
  logger.log() << "Reset the key in the keychain";

  if (s_initialized) {
    CredDeleteW(CRED_KEY, CRED_TYPE_GENERIC, 0);
    s_initialized = false;
  }
}

bool CryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (!s_initialized) {
    logger.log() << "Get key";

    s_initialized = true;

    {
      PCREDENTIALW cred;
      if (CredReadW(CRED_KEY, CRED_TYPE_GENERIC, 0, &cred)) {
        s_key =
            QByteArray((char*)cred->CredentialBlob, cred->CredentialBlobSize);
        logger.log() << "Key found with length:" << s_key.length();

        if (s_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
          memcpy(key, s_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
          return true;
        }
      } else if (GetLastError() != ERROR_NOT_FOUND) {
        logger.error() << "Failed to retrieve the key";
        return false;
      }
    }

    logger.log() << "Key not found. Let's create it.";
    s_key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
    QRandomGenerator* rg = QRandomGenerator::system();
    for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
      s_key[i] = rg->generate() & 0xFF;
    }

    {
      CREDENTIALW cred;
      memset(&cred, 0, sizeof(cred));

      cred.Comment = const_cast<wchar_t*>(CRED_KEY);
      cred.Type = CRED_TYPE_GENERIC;
      cred.TargetName = const_cast<wchar_t*>(CRED_KEY);
      cred.CredentialBlobSize = s_key.length();
      cred.CredentialBlob = (LPBYTE)s_key.constData();
      cred.Persist = CRED_PERSIST_ENTERPRISE;

      if (!CredWriteW(&cred, 0)) {
        logger.error() << "Failed to write the key";
        return false;
      }
    }
  }

  if (s_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(key, s_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }

  logger.log() << "Invalid key";
  return false;
}

// static
CryptoSettings::Version CryptoSettings::getSupportedVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
