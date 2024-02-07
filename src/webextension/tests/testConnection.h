/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QList>
#include <QObject>
#include <QTest>

class QByteArray;
class QIODevice;
class QJsonObject;

class TestConnection final : public QObject {
  Q_OBJECT

  // Helpers
  static void writeTo(const QByteArray& data, QIODevice* target);
  static void writeTo(const QByteArray& data, int len, QIODevice* target);

  static QList<QJsonObject> findObjects(const QByteArray);

 private slots:
  void init();
  void cleanup();
  void testEmptyBuffer();
  void testZeroSized();
  void testEmitsJSONMessages();
  void testInvalidJSONEmitsInvalid();
};
