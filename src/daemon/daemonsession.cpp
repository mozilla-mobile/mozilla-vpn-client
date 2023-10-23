/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonsession.h"

#if defined(MZ_MACOS)
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <QLocalSocket>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DaemonSession");
}  // namespace

DaemonSession::DaemonSession() { MZ_COUNT_CTOR(DaemonSession); }

DaemonSession::~DaemonSession() { MZ_COUNT_DTOR(DaemonSession); }

bool DaemonSession::start(QLocalSocket* socket) {
  if (isActive()) {
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

void DaemonSession::reset() {
  logger.debug() << "Session reset";
  m_sessionOwner = -1;
}

bool DaemonSession::isPeerAuthorized(QLocalSocket* socket) const {
  if (!isActive()) {
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
int DaemonSession::getPeerId(QLocalSocket* socket) {
#if defined(MZ_MACOS)
  Q_ASSERT(socket);

  uid_t uid;
  gid_t gid;
  int result = getpeereid(socket->socketDescriptor(), &uid, &gid);
  if (result == -1) {
    logger.error() << "Unable to get peer id. Error:" << errno;
    return -1;
  }

  return uid;
#endif

  // For platforms that don't support getpeereid,
  // we default to `root` which is always authorized.
  return 0;
}
