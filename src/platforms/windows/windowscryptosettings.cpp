/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscryptosettings.h"

#include <Windows.h>
#include <wincred.h>
#include <wincrypt.h>

#include <QRandomGenerator>

#include "constants.h"
#include "cryptosettings.h"
#include "logger.h"

namespace {
Logger logger("WindowsCryptoSettings");
}  // namespace

void WindowsCryptoSettings::resetKey() {
  logger.debug() << "Reset the key in the keychain";
  if (m_initialized) {
    CredDeleteW(Constants::WINDOWS_CRED_KEY, CRED_TYPE_GENERIC, 0);
    m_initialized = false;
  }
}

bool WindowsCryptoSettings::getKey(uint8_t key[CRYPTO_SETTINGS_KEY_SIZE]) {
  if (!m_initialized) {
    logger.debug() << "Get key";

    m_initialized = true;

    {
      PCREDENTIALW cred;
      if (CredReadW(Constants::WINDOWS_CRED_KEY, CRED_TYPE_GENERIC, 0, &cred)) {
        m_key =
            QByteArray((char*)cred->CredentialBlob, cred->CredentialBlobSize);
        logger.debug() << "Key found with length:" << m_key.length();

        if (m_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
          memcpy(key, m_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
          return true;
        }
      } else if (GetLastError() != ERROR_NOT_FOUND) {
        logger.error() << "Failed to retrieve the key";
        return false;
      }
    }

    logger.debug() << "Key not found. Let's create it.";
    m_key = QByteArray(CRYPTO_SETTINGS_KEY_SIZE, 0x00);
    QRandomGenerator* rg = QRandomGenerator::system();
    for (int i = 0; i < CRYPTO_SETTINGS_KEY_SIZE; ++i) {
      m_key[i] = rg->generate() & 0xFF;
    }

    {
      CREDENTIALW cred;
      memset(&cred, 0, sizeof(cred));

      cred.Comment = const_cast<wchar_t*>(Constants::WINDOWS_CRED_KEY);
      cred.Type = CRED_TYPE_GENERIC;
      cred.TargetName = const_cast<wchar_t*>(Constants::WINDOWS_CRED_KEY);
      cred.CredentialBlobSize = m_key.length();
      cred.CredentialBlob = (LPBYTE)m_key.constData();
      cred.Persist = CRED_PERSIST_ENTERPRISE;

      if (!CredWriteW(&cred, 0)) {
        logger.error() << "Failed to write the key";
        return false;
      }
    }
  }

  if (m_key.length() == CRYPTO_SETTINGS_KEY_SIZE) {
    memcpy(key, m_key.data(), CRYPTO_SETTINGS_KEY_SIZE);
    return true;
  }

  logger.warning() << "Invalid key";
  return false;
}

// static
CryptoSettings::Version WindowsCryptoSettings::getSupportedVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
