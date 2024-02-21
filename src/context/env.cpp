/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "env.h"

#include <QQuickWindow>
#include <QTextStream>

#include "qmlengineholder.h"

namespace {

#ifdef UNIT_TEST
bool s_debugMode =
#  ifdef MZ_DEBUG
    true;
#  else
    false;
#  endif
#endif

}  // namespace

// static
Env* Env::instance() {
  static Env env;
  return &env;
}

// static
QString Env::devVersion() {
  QString out;
  QTextStream stream(&out);

  stream << "Qt version: <b>";
  stream << qVersion();
  stream << "</b> - compiled: <b>";
  stream << QT_VERSION_STR;
  stream << "</b>";

  return out;
}

// static
QString Env::graphicsApi() {
  QQuickWindow* window =
      qobject_cast<QQuickWindow*>(QmlEngineHolder::instance()->window());
  if (!window) {
    // In unit-tests we do not have a window.
    return "unknown";
  }

  switch (window->rendererInterface()->graphicsApi()) {
    case QSGRendererInterface::Software:
      return "software";
    case QSGRendererInterface::OpenVG:
    case QSGRendererInterface::OpenGL:
      return "openGL/openVG";
    case QSGRendererInterface::Direct3D11:
      return "Direct3D11";
    case QSGRendererInterface::Vulkan:
      return "Vulkan";
    case QSGRendererInterface::Metal:
      return "Metal";
    case QSGRendererInterface::Unknown:
    case QSGRendererInterface::Null:
    default:
      return "unknown";
  }
}

// static
bool Env::debugMode() {
#ifdef UNIT_TEST
  return s_debugMode;
#endif

#ifdef MZ_DEBUG
  return true;
#else
  return false;
#endif
}

#ifdef UNIT_TEST
void Env::setDebugMode(bool debugMode) {
  s_debugMode = debugMode;
  emit debugModeChanged();
}
#endif

// static
bool Env::stagingMode() { return !Constants::inProduction(); }

#ifdef UNIT_TEST
void Env::setStagingMode(bool stagingMode) {
  if (stagingMode) {
    Constants::setStaging();
  }
  emit stagingModeChanged();
}
#endif

#ifdef MZ_WINDOWS
// static
QString Env::windowsVersion() {
  QSettings regCurrentVersion(
      "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
      QSettings::NativeFormat);

  int buildNr = regCurrentVersion.value("CurrentBuild").toInt();
  if (buildNr >= WINDOWS_11_BUILD) {
    return "11";
  }
  return QSysInfo::productVersion();
}
#endif

QString Env::osVersion() {
#ifdef MZ_WINDOWS
  return windowsVersion();
#else
  return QSysInfo::productVersion();
#endif
}
