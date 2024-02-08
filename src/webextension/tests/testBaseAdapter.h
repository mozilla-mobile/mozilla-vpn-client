/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#ifndef TEST_BASE_ADAPTER_H
#  define TEST_BASE_ADAPTER_H

class TestBaseAdapter final : public QObject {
  Q_OBJECT

 private slots:
  void testOnMessage();
};

#endif
