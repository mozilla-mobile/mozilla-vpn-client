/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestComposer final : public TestHelper {
  Q_OBJECT

 private slots:
  void generic_data();
  void generic();

  void button_data();
  void button();

  void text_data();
  void text();

  void title_data();
  void title();

  void unorderedList_data();
  void unorderedList();

  void orderedList_data();
  void orderedList();
};
