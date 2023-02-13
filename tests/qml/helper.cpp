/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

#include "glean.h"
#include "gleandeprecated.h"
#include "nebula.h"
#include "qmlengineholder.h"

TestHelper::TestHelper() {
  m_i18nstrings = I18nStrings::instance();
  m_theme = new Theme();
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
  emit GleanDeprecated::instance()->recordGleanEvent(event);
}

void TestHelper::triggerRecordGleanEventWithExtraKeys(
    const QString& event, const QVariantMap& keys) const {
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(event, keys);
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
  Nebula::Initialize(engine);
  Glean::Initialize(engine);
  engine->addImportPath("qrc:///");

  if (!QmlEngineHolder::exists()) {
    new QmlEngineHolder(engine);
  } else {
    QmlEngineHolder::instance()->replaceEngine(engine);
  }

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
      "Mozilla.VPN", 1, 0, "VPNI18n",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_i18nstrings;
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
      "Mozilla.VPN", 1, 0, "VPNTheme",
      [this](QQmlEngine*, QJSEngine* engine) -> QObject* {
        m_theme->initialize(engine);
        QObject* obj = m_theme;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "MZGleanDeprecated",
      [](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = GleanDeprecated::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });
};
