/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonaccesscontrol.h"

#if defined(MZ_MACOS)
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <QLocalSocket>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DaemonAccessControl");
}  // namespace

DaemonAccessControl::DaemonAccessControl() {
  MZ_COUNT_CTOR(DaemonAccessControl);
}

DaemonAccessControl::~DaemonAccessControl() {
  MZ_COUNT_DTOR(DaemonAccessControl);
}

bool DaemonAccessControl::isSessionActive() const {
  return m_sessionOwner != -1;
}

bool DaemonAccessControl::startSession(QLocalSocket* socket) {
  if (isSessionActive()) {
    return false;
  }

  int peerId = getPeerId(socket);
  if (peerId == -1) {
    return false;
  }

  logger.debug() << "Session started by user" << peerId;
  m_sessionOwner = peerId;
  return true;
}

void DaemonAccessControl::resetSession() {
  if (!isSessionActive()) {
    return;
  }

  logger.debug() << "Session reset";
  m_sessionOwner = -1;
}

bool DaemonAccessControl::authorizeCommandForPeer(const QString& command,
                                                  QLocalSocket* socket) {
  if (!isSessionActive()) {
    if (command == "activate") {
      if (!startSession(socket)) {
        logger.error() << "Unable to start new session."
                          "Can't authorize `activate` command.";

        return false;
      }
    } else if (command == "status") {
      // Other than "activate",
      // only "status" command is available when there is no ongoing session.
      return true;
    }

    logger.error() << "Attempted to authorize command:" << command
                   << "but there is no ongoing session. Unauthorized.";
    return false;
  }

  if (!isPeerAuthorized(socket)) {
    logger.warning() << "Caller is unauthorized.";
    return false;
  }

  return true;
}

bool DaemonAccessControl::isPeerAuthorized(QLocalSocket* socket) const {
  if (!isSessionActive()) {
    logger.debug() << "Unable to determine if peer is authorized."
                      "Session must be started first."
                      "Defaulting to unauthorized.";
    return false;
  }

  int uid = getPeerId(socket);
  if (uid == -1) {
    logger.error() << "Unable to determine if peer is authorized."
                      "Assuming unauthorized.";
    return false;
  }

  if (uid == 0) {
    logger.debug()
        << "Verifying authorization for peer and peer seems to be `root`."
           "`root` is always authorized.";
    return true;
  }

  return m_sessionOwner == uid;
}

// static
int DaemonAccessControl::getPeerId(QLocalSocket* socket) {
#if defined(MZ_MACOS)
  Q_ASSERT(socket);

  uid_t uid = -1;
  gid_t gid = -1;
  int result = getpeereid(socket->socketDescriptor(), &uid, &gid);
  if (result == -1) {
    logger.error() << "Unable to get peer id. Error:" << errno;
    return -1;
  }

  if (uid == -1 || gid == -1) {
    logger.error() << "Unable to get peer id. Unknown error.";
    return -1;
  }

  return uid;
#endif

  // For platforms that don't support getpeereid,
  // we default to `root` which is always authorized.
  return 0;
}
