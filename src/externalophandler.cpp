/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "externalophandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "frontend/navigator.h"

#include <QCoreApplication>

namespace {
ExternalOpHandler* s_instance = nullptr;
Logger logger(LOG_MAIN, "ExternalOpHandler");
}  // namespace

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

void ExternalOpHandler::registerBlocker(Blocker* blocker) {
  Q_ASSERT(blocker);
  Q_ASSERT(!m_blockers.contains(blocker));

  logger.debug() << "Blocker registered";
  m_blockers.append(blocker);
}

void ExternalOpHandler::unregisterBlocker(Blocker* blocker) {
  Q_ASSERT(blocker);
  Q_ASSERT(m_blockers.contains(blocker));

  logger.debug() << "Blocker unregistered";
  m_blockers.removeOne(blocker);
}

void ExternalOpHandler::request(Op op) {
  logger.debug() << "Op request received";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  for (Blocker* blocker : m_blockers) {
    if (blocker->maybeBlockRequest(op)) {
      logger.debug() << "Operation rejected by a blocker";
      return;
    }
  }

  switch (op) {
    case OpAbout:
      vpn->requestAbout();
      break;
    case OpActivate:
      vpn->controller()->activate();
      break;
    case OpDeactivate:
      vpn->controller()->deactivate();
      break;
    case OpGetHelp:
      vpn->requestGetHelp();
      break;
    case OpSettings:
      vpn->requestSettings();
      break;
    case OpQuit:
      vpn->controller()->quit();
      break;

    // List of no-op requests:
    case OpNotificationClicked:
      [[fallthrough]];
    case OpCloseEvent:
      break;
  }
}
