/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "../../src/mozillavpn.h"
#include "../../src/settingsholder.h"
#include "l18nstrings.h"
#include "helper.h"

TestHelper::TestHelper() {
  FeatureList::instance()->initialize();
  m_whatsNewModel = new WhatsNewModel();
}

void TestHelper::triggerInitializeGlean() {
  emit MozillaVPN::instance()->initializeGlean();
}

void TestHelper::triggerSetGleanSourceTags(const QStringList& tags) {
  emit MozillaVPN::instance()->setGleanSourceTags(tags);
}

void TestHelper::triggerAboutToQuit() {
  emit MozillaVPN::instance()->aboutToQuit();
}

// The next two functions duplicate mozillavpn.h logic, but I'm not
// sure how else to generate an "expected" value for tst_mainWindowGlean.qml
// as it needs to be dynamic based on the platform people are running tests on.
// At the least this should fail give someone pause for thought if they change
// the mozillavpn.h logic.
QString TestHelper::osVersion() { return QSysInfo::productVersion(); }
QString TestHelper::architecture() {
  return QSysInfo::currentCpuArchitecture();
}

void TestHelper::qmlEngineAvailable(QQmlEngine* engine) {
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
      "Mozilla.VPN", 1, 0, "VPNl18n", [](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = L18nStrings::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNSettings",
      [](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = SettingsHolder::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<TestHelper>(
      "TestHelper", 1, 0, "TestHelper",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = this;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNCloseEventHandler",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = &this->closeEventHandler;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNWhatsNewModel",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = this->whatsNewModel();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });
};
