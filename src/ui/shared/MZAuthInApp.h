/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "authenticationinapp/authenticationinapp.h"

struct MZAuthInApp {
  Q_GADGET
  QML_FOREIGN(AuthenticationInApp)
  QML_NAMED_ELEMENT(MZAuthInApp)
  QML_SINGLETON

 public:
  static AuthenticationInApp* create(QQmlEngine*, QJSEngine*) {
    return AuthenticationInApp::instance();
  }
};
