/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QSettings>

#include "cryptosettings.h"
#include "helper.h"

class QTemporaryDir;

class TestCryptoSettings final : public TestHelper {
  Q_OBJECT

 private:
  void writeTestData(CryptoSettings& crypto);
  void checkTestData(const QSettings& settings);
  QString testFileName() const { return m_tempdir->filePath("settings.moz"); };

  // Parse content from encrypted files.
  QByteArray parseHeader() const;
  CryptoSettings::Version parseVersion() const;
  QByteArray parseMac() const;
  QByteArray parseNonce() const;
  QByteArray parseMetaData() const;

  QTemporaryDir* m_tempdir;

 private slots:
  void init();
  void cleanup();

  void readAndWrite();
  void resetKeyOnRollover();
  void restoreNonceFromFile();
  void readWritePlaintext();
  void readFailsWithPadding();
  void readFailsWithMacError();

  // Tests for the encrypted v2 format.
  void writeV1readV2upgrade();
  void writeV2WithMetaData();
  void readFailsWithMetaDataError();
};
