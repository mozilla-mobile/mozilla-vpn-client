/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cryptosettings.h"
#include "helper.h"

#include <QSettings>

class QTemporaryFile;

class TestCryptoSettings final : public TestHelper {
  Q_OBJECT

 private:
  void writeTestData(CryptoSettings& crypto);
  void checkTestData(const QSettings& settings);
  CryptoSettings::Version parseVersion() const;

  QTemporaryFile* m_tempfile = nullptr;

 private slots:
  void init();
  void cleanup();

  void readAndWrite();
  void resetKeyOnRollover();
  void readWritePlaintext();
  void readFailsWithPadding();
  void readFailsWithMacError();
};
