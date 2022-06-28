/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "externalophandler.h"
#include "leakdetector.h"
#include "mozillavpn.h"

#include <QCoreApplication>

namespace {
ExternalOpHandler* s_instance = nullptr;
}

// static
ExternalOpHandler* ExternalOpHandler::instance() {
  if (!s_instance) {
    s_instance = new ExternalOpHandler(qApp);
  }
  return s_instance;
}

ExternalOpHandler::ExternalOpHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(ExternalOpHandler);
}

ExternalOpHandler::~ExternalOpHandler() { MVPN_COUNT_DTOR(ExternalOpHandler); }

void ExternalOpHandler::request(Op op) {
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  emit requestReceived(op);

  switch (op) {
    case OpContactUs:
      vpn->requestContactUs();
      break;
    case OpViewLogs:
      vpn->requestViewLogs();
      break;
    case OpActivate:
      vpn->controller()->activate();
      break;
    case OpDeactivate:
      vpn->controller()->deactivate();
      break;
    case OpSettings:
      vpn->requestSettings();
      break;
    case OpQuit:
      vpn->controller()->quit();
      break;
    case OpNotificationClicked:
      break;
  }
}
