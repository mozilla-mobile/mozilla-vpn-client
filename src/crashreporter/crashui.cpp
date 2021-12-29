/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashui.h"
#include "fontloader.h"
#include "imageproviderfactory.h"
#include "nebula.h"
#include "theme.h"

#include <QCoreApplication>
#include <QWindow>
#include "l18nstrings.h"

using namespace std;

constexpr auto APP = "app";
constexpr auto QML_MAIN = "qrc:/crashui/main.qml";

CrashUI::CrashUI() { m_engine = make_unique<QQmlApplicationEngine>(); }

void CrashUI::initialize() {
  if (!m_initialized) {
    Nebula::Initialize(m_engine.get());
    FontLoader::loadFonts();
    auto provider = ImageProviderFactory::create(QCoreApplication::instance());
    if (provider) {
      m_engine->addImageProvider(APP, provider);
    }

    qmlRegisterSingletonType<L18nStrings>(
        "Mozilla.VPN", 1, 0, "VPNl18n",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = L18nStrings::instance();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
    m_theme = make_shared<Theme>();
    m_theme->loadThemes();
    qmlRegisterSingletonType<Theme>(
        "Mozilla.VPN", 1, 0, "VPNTheme",
        [this](QQmlEngine*, QJSEngine*) -> QObject* {
          QObject* obj = m_theme.get();
          QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
          return obj;
        });
    const QUrl url(QML_MAIN);
    m_engine->load(url);
    m_initialized = true;
  }
}

void CrashUI::showUI() {
  auto root = m_engine->rootObjects().first();
  QWindow* window = qobject_cast<QWindow*>(root);
  window->show();
  window->raise();
  window->requestActivate();
}