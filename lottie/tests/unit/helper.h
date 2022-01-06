/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QtTest/QtTest>
#include <QVector>

class TestHelper : public QObject {
  Q_OBJECT

 public:
  TestHelper();

 public:
  static QVector<QObject*> testList;
};

#endif  // HELPER_H
