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

TestHelper* TestHelper::instance() {
  static TestHelper* s_instance = new TestHelper();
  return s_instance;
}

bool TestHelper::stagingMode() { return m_stagingMode; }

void TestHelper::setStagingMode(bool stagingMode) {
  m_stagingMode = stagingMode;
}

void TestHelper::triggerInitializeGlean() {
  emit MozillaVPN::instance()->initializeGlean();
}

void TestHelper::triggerSetGleanSourceTags(const QStringList& tags) {
  emit MozillaVPN::instance()->setGleanSourceTags(tags);
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
