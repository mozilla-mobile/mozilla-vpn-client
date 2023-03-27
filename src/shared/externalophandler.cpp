/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "externalophandler.h"

#include <QCoreApplication>

#include "leakdetector.h"
#include "logger.h"

namespace {
ExternalOpHandler* s_instance = nullptr;
Logger logger("ExternalOpHandler");
}  // namespace

// static
ExternalOpHandler* ExternalOpHandler::instance() {
  if (!s_instance) {
    s_instance = new ExternalOpHandler(qApp);
  }
  return s_instance;
}

ExternalOpHandler::ExternalOpHandler(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(ExternalOpHandler);
}

ExternalOpHandler::~ExternalOpHandler() { MZ_COUNT_DTOR(ExternalOpHandler); }

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

bool ExternalOpHandler::request(int op) {
  logger.debug() << "Op request received" << op;

  for (Blocker* blocker : m_blockers) {
    if (blocker->maybeBlockRequest(op)) {
      logger.debug() << "Operation rejected by a blocker";
      return false;
    }
  }

  Q_ASSERT(m_ops.contains(op));
  if (!m_ops.contains(op)) {
    logger.debug() << "Operation unknown.";
    return false;
  }

  m_ops[op]();
  return true;
}

void ExternalOpHandler::registerExternalOperation(int op, void (*callback)()) {
  logger.debug() << "Registering" << op;
  m_ops.insert(op, callback);
}
