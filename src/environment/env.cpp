/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "env.h"

#include <QTextStream>

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
