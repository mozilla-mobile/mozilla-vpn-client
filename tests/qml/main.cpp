/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QtQuickTest>

#include "helper.h"

// static
App* App::instance() {
  static App* app = nullptr;

  if (!app) {
    app = new App(qApp);
  }

  return app;
}

int main(int argc, char* argv[]) {
  QCoreApplication::setApplicationName("Mozilla VPN QML Tests");
  QCoreApplication::setOrganizationName("Mozilla Testing");
  QCoreApplication::setApplicationVersion(Constants::versionString());

  // In order to be able to call our TestHelper constructor how we want,
  // we do not follow the exact instructions here
  // https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
  // but instead call the components of the very simple macro ourselves.
  // Macro source code:
  // https://github.com/qt/qtdeclarative/blob/5.15.2/src/qmltest/quicktest.h#L69
  QTEST_SET_MAIN_SOURCE_PATH
  return quick_test_main_with_setup(
      argc, argv, "qml_tests", QUICK_TEST_SOURCE_DIR, TestHelper::instance());
}
