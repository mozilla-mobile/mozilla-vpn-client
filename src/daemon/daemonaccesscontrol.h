/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMONSESSION_H
#define DAEMONSESSION_H

#include <QString>

class QLocalSocket;

/**
 * @brief Controls access for peers that are attempting to send comments through
 * the daemon socket.
 *
 * Access is controlled through sessions. A session can be activated at any time
 * and only a single session may be active at a time. Each session is attached
 * to a "session owner". While a session is ongoing only the owner (or root) is
 * allowed to send commands to the daemon.
 */
class DaemonAccessControl final {
 public:
  explicit DaemonAccessControl();
  ~DaemonAccessControl();

  static DaemonAccessControl* instance();

  void resetSession();
  bool isCommandAuthorizedForPeer(const QString& command,
                                  const QLocalSocket* socket);

#ifdef UNIT_TEST
  static inline void setPeerId(int peerId) { s_mockPeerId = peerId; }
#endif

 private:
  bool isSessionActive() const;
  bool startSession(const QLocalSocket* socket);

  static int getPeerId(const QLocalSocket* socket);

  /**
   * @brief Only the peer that started the session or a peer that has
   * special permissions e.g. root, may modify the currently ongoing
   * session.
   */
  bool isPeerAuthorized(const QLocalSocket* socket) const;

 private:
  // The id of the user who started the currently active session.
  //
  // A value of -1 means there is currently no ongoing session.
  int m_sessionOwner = -1;

#ifdef UNIT_TEST
  int static inline s_mockPeerId = -1;
#endif
};

#endif  // DAEMONSESSION_H
