/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testresourceloader.h"
#include "../../src/resourceloader.h"
#include "../../src/resourceloaderutils.h"

#include <QTemporaryDir>

namespace {
QStringList fileInFolder(const QString& path) {
  QDir dir(path);
  if (!dir.exists()) {
    return QStringList{"__invalid__"};
  }

  QStringList files = dir.entryList();
  files.removeOne(".");
  files.removeOne("..");

  files.sort();
  return files;
}

void addUint32ToQByteArray(QByteArray& array, uint32_t value) {
  char* rawValue = reinterpret_cast<char*>(&value);
  array.append(rawValue, sizeof(uint32_t));
}

}  // namespace

void TestResourceLoader::fileNameLength_data() {
  QTest::addColumn<int32_t>("length");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << 0 << false;
  QTest::addRow("too big") << 31 << false;
  QTest::addRow("ok") << 10 << true;
}

void TestResourceLoader::fileNameLength() {
  QFETCH(int32_t, length);
  QFETCH(bool, result);

  QCOMPARE(ResourceLoaderUtils::isFileNameLengthValid(length), result);
}

void TestResourceLoader::fileName_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << "" << false;
  QTest::addRow("too big") << "1234567890123456789012345678901" << false;
  QTest::addRow("starting with dot") << ".foo" << false;
  QTest::addRow("slash") << "foo/bar" << false;
  QTest::addRow("backslash") << "foo\\bar" << false;
  QTest::addRow("invalid char") << "foo$bar" << false;
  QTest::addRow("all good") << "f_o-o.BAR.1234567890" << true;
}

void TestResourceLoader::fileName() {
  QFETCH(QString, input);
  QFETCH(bool, result);

  QCOMPARE(ResourceLoaderUtils::isFileNameValid(input), result);
}

void TestResourceLoader::cleanupCacheFolder() {
  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    return;
  }

  QDir dir(tmpDir.path());

  // Empty folder:
  QVERIFY(ResourceLoader::cleanupCacheFolder(dir));
  QVERIFY(dir.exists("qrb_temp"));
  QCOMPARE(fileInFolder(dir.path()), QStringList{"qrb_temp"});
  QVERIFY(fileInFolder(dir.filePath("qrb_temp")).isEmpty());

  // With a journal folder
  QVERIFY(dir.mkdir("qrb_journal"));
  QVERIFY(ResourceLoader::cleanupCacheFolder(dir));
  QVERIFY(dir.exists("qrb_temp"));
  QCOMPARE(fileInFolder(dir.path()), QStringList{"qrb_temp"});
  QVERIFY(fileInFolder(dir.filePath("qrb_temp")).isEmpty());

  // Something in the temp folder
  {
    QFile file(QDir(dir.filePath("qrb_temp")).filePath("foo"));
    QVERIFY(file.open(QFile::WriteOnly));
  }
  QCOMPARE(fileInFolder(dir.filePath("qrb_temp")), QStringList{"foo"});
  QVERIFY(ResourceLoader::cleanupCacheFolder(dir));
  QVERIFY(dir.exists("qrb_temp"));
  QCOMPARE(fileInFolder(dir.path()), QStringList{"qrb_temp"});
  QVERIFY(fileInFolder(dir.filePath("qrb_temp")).isEmpty());
}

void TestResourceLoader::deserializeData_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QStringList>("list");
  QTest::addColumn<QStringList>("content");

  QByteArray input;
  QTest::addRow("empty") << input << false;

  input.append("MV");
  QTest::addRow("invalid header") << input << false;

  input.append("PN");
  QTest::addRow("no version") << input << false;

  input.append(QByteArray(1, 0x99));
  QTest::addRow("invalid version") << input << false;

  input.replace(4, 1, QByteArray(1, 0x01));
  QTest::addRow("good but empty")
      << input << true << QStringList{} << QStringList{};

  input.append(QByteArray(1, 0x12));
  QTest::addRow("invalid file name size") << input << false;

  input.remove(input.length() - 1, 1);
  addUint32ToQByteArray(input, 0);
  QTest::addRow("invalid file name size - 0") << input << false;

  input.remove(input.length() - 4, 4);
  addUint32ToQByteArray(input, 31);
  QTest::addRow("invalid file name size - 31") << input << false;

  input.remove(input.length() - 4, 4);
  addUint32ToQByteArray(input, 3);
  QTest::addRow("invalid file name") << input << false;

  input.append("foo");
  QTest::addRow("invalid content size - missing") << input << false;

  input.append(QByteArray(1, 0x12));
  QTest::addRow("invalid content size - incompleted") << input << false;

  input.remove(input.length() - 1, 1);
  addUint32ToQByteArray(input, 6);
  QTest::addRow("invalid content") << input << false;

  input.append("hello!");
  QTest::addRow("good with 1 file")
      << input << true << QStringList{"foo"} << QStringList{"hello!"};

  addUint32ToQByteArray(input, 3);
  input.append("bar");
  addUint32ToQByteArray(input, 6);
  input.append("world!");
  QTest::addRow("good with 2 files")
      << input << true << QStringList{"bar", "foo"}
      << QStringList{"world!", "hello!"};
}

void TestResourceLoader::deserializeData() {
  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    return;
  }

  QDir dir(tmpDir.path());

  QVERIFY(dir.mkdir("qrb_temp"));
  QFETCH(QByteArray, input);
  QFETCH(bool, result);

  QCOMPARE(ResourceLoader::deserializeData(input, dir), result);
  if (result) {
    QFETCH(QStringList, list);
    QCOMPARE(fileInFolder(dir.filePath("qrb_temp")), list);

    QFETCH(QStringList, content);
    QCOMPARE(list.length(), content.length());

    QDir tmpDir(dir.filePath("qrb_temp"));
    for (int i = 0; i < list.length(); ++i) {
      QFile file(tmpDir.filePath(list[i]));
      QVERIFY(file.open(QFile::ReadOnly));
      QCOMPARE(file.readAll(), content[i]);
    }
  }
}

void TestResourceLoader::bringToProd() {
  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    return;
  }

  QDir dir(tmpDir.path());

  // Just a temporary folder:
  QVERIFY(dir.mkdir("qrb_temp"));
  {
    QFile file(QDir(dir.filePath("qrb_temp")).filePath("foo"));
    QVERIFY(file.open(QFile::WriteOnly));
  }
  QVERIFY(ResourceLoader::bringToProd(dir));
  QCOMPARE(fileInFolder(dir.path()), QStringList{"qrb"});
  QCOMPARE(fileInFolder(dir.filePath("qrb")), QStringList{"foo"});

  // Just a prod + temporary folder:
  QVERIFY(dir.mkdir("qrb_temp"));
  {
    QFile file(QDir(dir.filePath("qrb_temp")).filePath("bar"));
    QVERIFY(file.open(QFile::WriteOnly));
  }
  QVERIFY(ResourceLoader::bringToProd(dir));
  QCOMPARE(fileInFolder(dir.path()), QStringList{"qrb"});
  QCOMPARE(fileInFolder(dir.filePath("qrb")), QStringList{"bar"});
}

// TODO: manifest validation

static TestResourceLoader s_testResourceLoader;
