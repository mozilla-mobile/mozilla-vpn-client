/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

#include "nebula.h"
#include "qmlengineholder.h"
#include "settings/settinggroup.h"

TestHelper::TestHelper() {
  m_i18nstrings = I18nStrings::instance();
  m_mozillavpn = MozillaVPN::instance();

  m_testSettingGroup = new SettingGroup("aGroup");
  m_testSettingGroup->set("aKey", QVariant("Hello, QML!"));
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

void TestHelper::qmlEngineAvailable(QQmlEngine* engine) {
  Nebula::Initialize(engine);
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
      "Mozilla.Shared", 1, 0, "MZI18n",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_i18nstrings;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<TestHelper>(
      "Mozilla.Shared", 1, 0, "MZSettings",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = &m_settingsHolder;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.Shared", 1, 0, "MZTheme",
      [](QQmlEngine*, QJSEngine* engine) -> QObject* {
        Theme::instance()->initialize(engine);
        QObject* obj = Theme::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<SettingGroup>(
      "Mozilla.Shared", 1, 0, "TestSettingGroup",
      [this](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = m_testSettingGroup;
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });
};
