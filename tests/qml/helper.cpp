/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

TestHelper::TestHelper() {
  FeatureList::instance()->initialize();
  m_closeEventHandler = new CloseEventHandler();
  m_whatsNewModel = new WhatsNewModel();
  m_l18nstrings = L18nStrings::instance();
  m_mozillavpn = MozillaVPN::instance();
}

TestHelper* TestHelper::instance() {
  static TestHelper* s_instance = new TestHelper();
  return s_instance;
}

bool TestHelper::mainWindowLoadedCalled() const {
  return m_mainWindowLoadedCalled;
}

void TestHelper::setMainWindowLoadedCalled(bool val) {
  m_mainWindowLoadedCalled = val;
}

bool TestHelper::stagingMode() const { return m_stagingMode; }

void TestHelper::setStagingMode(bool val) { m_stagingMode = val; }

bool TestHelper::debugMode() const { return m_debugMode; }

void TestHelper::setDebugMode(bool val) { m_debugMode = val; }

void TestHelper::triggerInitializeGlean() const {
  emit MozillaVPN::instance()->initializeGlean();
}

void TestHelper::triggerRecordGleanEvent(const QString& event) const {
  emit MozillaVPN::instance()->recordGleanEvent(event);
}

void TestHelper::triggerSendGleanPings() const {
  emit MozillaVPN::instance()->sendGleanPings();
}

void TestHelper::triggerSetGleanSourceTags(const QStringList& tags) const {
  emit MozillaVPN::instance()->setGleanSourceTags(tags);
}

void TestHelper::triggerAboutToQuit() const {
  emit MozillaVPN::instance()->aboutToQuit();
}

void TestHelper::qmlEngineAvailable(QQmlEngine* engine) {
  engine->addImportPath("qrc:///compat");
  engine->addImportPath("qrc:///components");
  engine->addImportPath("qrc:///glean");
  engine->addImportPath("qrc:///themes");
  engine->addImportPath("qrc:///");

  qmlRegisterSingletonType<TestHelper>(
      "TestHelper", 1, 0, "TestHelper",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = this;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPN", [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_mozillavpn;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNl18n",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_l18nstrings;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<TestHelper>(
      "Mozilla.VPN", 1, 0, "VPNSettings",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = &m_settingsHolder;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNCloseEventHandler",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_closeEventHandler;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "VPNWhatsNewModel",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_whatsNewModel;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });
};
