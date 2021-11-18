/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>

#include "l18nstrings.h"
#include "../../src/mozillavpn.h"
#include "../../src/settingsholder.h"
#include "helper.h"

// For info on the slots we can use
// https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
class Setup : public QObject {
  Q_OBJECT

 private:
  SettingsHolder settingsHolder;

 public:
  Setup() {}

 public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    FeatureList::instance()->initialize();

    engine->addImportPath("qrc:///compat");
    engine->addImportPath("qrc:///components");
    engine->addImportPath("qrc:///glean");
    engine->addImportPath("qrc:///themes");
    engine->addImportPath("qrc:///");

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPN", [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = MozillaVPN::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNl18n",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = L18nStrings::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNSettings",
        [this](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = &this->settingsHolder;
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNCloseEventHandler",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = TestHelper::instance()->closeEventHandler();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<MozillaVPN>(
        "Mozilla.VPN", 1, 0, "VPNWhatsNewModel",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = TestHelper::instance()->whatsNewModel();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });

    qmlRegisterSingletonType<TestHelper>(
        "TestHelper", 1, 0, "TestHelper",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = TestHelper::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
  }
};

// The name qml_tests is arbitrary
QUICK_TEST_MAIN_WITH_SETUP(qml_tests, Setup)

// I don't understand why we need this line, but the docs say we do:
// https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
#include "main.moc"
