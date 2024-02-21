/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

class TestSettingGroup final : public QObject {
  Q_OBJECT

 private slots:
  // This function is called after each test function
  void cleanup();

  // Not adding a generic test for `set` or `get`,
  // because that is tested by all other tests already.
  void testLoadSettingsOnInit();
  void testSettingsAreCreatedWithTheRightProperties();

  void testGetAndSetUnregisteredSetting();
  void testGetAndSetDisallowedSetting();

  void testRemove();
};
