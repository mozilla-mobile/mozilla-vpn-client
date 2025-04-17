/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>

#include "helper.h"

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <nativeMessagingApp>" << std::endl;
    return 1;
  }

  return TestHelper::runTests(argv[1]);
}
