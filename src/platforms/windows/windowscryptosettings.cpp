/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscryptosettings.h"

#include <Windows.h>
#include <wincred.h>
#include <wincrypt.h>

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

QByteArray WindowsCryptoSettings::getKey(CryptoSettings::Version version,
                                         const QByteArray& metadata) {
  Q_UNUSED(version);
  Q_UNUSED(metadata);

  if (!m_initialized) {
    logger.debug() << "Get key";

    m_initialized = true;

    {
      PCREDENTIALW cred;
      if (CredReadW(Constants::WINDOWS_CRED_KEY, CRED_TYPE_GENERIC, 0, &cred)) {
        m_key =
            QByteArray((char*)cred->CredentialBlob, cred->CredentialBlobSize);
        logger.debug() << "Key found with length:" << m_key.length();
        return m_key;
      } else if (GetLastError() != ERROR_NOT_FOUND) {
        logger.error() << "Failed to retrieve the key";
        return QByteArray();
      }
    }

    logger.debug() << "Key not found. Let's create it.";
    m_key = generateRandomBytes(CRYPTO_SETTINGS_KEY_SIZE);

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
        return QByteArray();
      }
    }
  }

  return m_key;
}

CryptoSettings::Version WindowsCryptoSettings::getPreferredVersion() {
  return CryptoSettings::EncryptionChachaPolyV1;
}
