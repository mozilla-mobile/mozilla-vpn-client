/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcryptosettings.h"

#include "platforms/dummy/dummycryptosettings.h"

void TestCryptoSettings::init() {
  cleanup();
  m_tempdir = new QTemporaryDir();
}

void TestCryptoSettings::cleanup() {
  if (m_tempdir) {
    delete m_tempdir;
    m_tempdir = nullptr;
  }
}

// Load some test data into the settings.
void TestCryptoSettings::writeTestData(CryptoSettings& crypto) {
  // Write some settings into one file.
  QSettings wSettings(m_tempdir->filePath("write.moz"), crypto.format());
  wSettings.setValue("someString", "Lorem Ipsum");
  wSettings.setValue("someInteger", 9001);
  wSettings.setValue("someBool", true);
  wSettings.sync();

  // Copy the file to the final location so that QSettings can't cache it.
  QFile file(m_tempdir->filePath("write.moz"));
  file.copy(testFileName());
}

void TestCryptoSettings::checkTestData(const QSettings& settings) {
  QCOMPARE(settings.value("someString"), "Lorem Ipsum");
  QCOMPARE(settings.value("someInteger"), 9001);
  QCOMPARE(settings.value("someBool"), true);
}

CryptoSettings::Version TestCryptoSettings::parseVersion() const {
  QFile file(testFileName());
  Q_ASSERT(file.open(QIODeviceBase::ReadOnly));

  QByteArray version = file.peek(1);
  return static_cast<CryptoSettings::Version>(version.at(0));
}

void TestCryptoSettings::readAndWrite() {
  DummyCryptoSettings crypto;

  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::EncryptionChachaPolyV1);

  QSettings rSettings(testFileName(), crypto.format());
  checkTestData(rSettings);
}

void TestCryptoSettings::resetKeyOnRollover() {
  DummyCryptoSettings crypto;
  QByteArray metadata;
  QByteArray initialKey = crypto.getKey(metadata);
  crypto.m_lastNonce = UINT64_MAX - 1;

  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::EncryptionChachaPolyV1);

  QByteArray finalKey = crypto.getKey(metadata);
  QVERIFY(crypto.m_lastNonce < 10);
  QVERIFY(initialKey != finalKey);
}

void TestCryptoSettings::restoreNonceFromFile() {
  DummyCryptoSettings crypto;
  crypto.m_lastNonce = 12345;
  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::EncryptionChachaPolyV1);
  QCOMPARE(crypto.m_lastNonce, 12346);

  crypto.m_lastNonce = 555;
  QSettings rSettings(testFileName(), crypto.format());
  checkTestData(rSettings);
  QCOMPARE(crypto.m_lastNonce, 12346);
}

void TestCryptoSettings::readWritePlaintext() {
  DummyCryptoSettings crypto;
  crypto.m_keyVersion = CryptoSettings::NoEncryption;

  // Write the settings as plaintext.
  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::NoEncryption);

  // Restore the key and read the settings.
  crypto.m_keyVersion = CryptoSettings::NoEncryption;
  QSettings rSettings(testFileName(), crypto.format());
  checkTestData(rSettings);
}

void TestCryptoSettings::readFailsWithPadding() {
  DummyCryptoSettings crypto;
  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::EncryptionChachaPolyV1);

  // Pad the file with an extra byte onto the end of the ciphertext.
  QFile file(testFileName());
  Q_ASSERT(file.open(QIODeviceBase::ReadWrite | QIODevice::Append));
  Q_ASSERT(file.putChar(0xb2));

  // Reading the file should fail due an AEAD authentication error.
  QSettings::SettingsMap map;
  Q_ASSERT(file.reset());
  QCOMPARE(CryptoSettings::readFile(file, map), false);
}

void TestCryptoSettings::readFailsWithMacError() {
  DummyCryptoSettings crypto;
  writeTestData(crypto);
  QCOMPARE(parseVersion(), CryptoSettings::EncryptionChachaPolyV1);

  // Encrypted v1 format: version | nonce | mac | ciphertext
  constexpr const qint64 encrypted_v1_mac_start = 13;
  QFile file(testFileName());
  Q_ASSERT(file.open(QIODeviceBase::ReadWrite));
  Q_ASSERT(file.seek(encrypted_v1_mac_start));

  // Flip a bit in the MAC
  char c;
  Q_ASSERT(file.getChar(&c));
  Q_ASSERT(file.seek(encrypted_v1_mac_start));
  Q_ASSERT(file.putChar(c & 0x04));
  Q_ASSERT(file.reset());

  // Reading the file should fail due an AEAD authentication error.
  QSettings::SettingsMap map;
  Q_ASSERT(file.reset());
  QCOMPARE(CryptoSettings::readFile(file, map), false);
}

static TestCryptoSettings s_testCryptoSettings;
