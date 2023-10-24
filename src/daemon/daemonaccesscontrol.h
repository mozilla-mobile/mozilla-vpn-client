/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMONSESSION_H
#define DAEMONSESSION_H

#include <QString>

class QLocalSocket;

class DaemonAccessControl final {
 public:
  explicit DaemonAccessControl();
  ~DaemonAccessControl();

  bool isSessionActive() const;

  bool startSession(QLocalSocket* socket = nullptr);
  void resetSession();

  bool authorizeCommandForPeer(const QString& command,
                               QLocalSocket* socket = nullptr);

 private:
  static int getPeerId(QLocalSocket* socket = nullptr);

  /**
   * @brief Only the peer that started the session or a peer that has
   * special permissions e.g. root, may modify the currently ongoing
   * session.
   */
  bool isPeerAuthorized(QLocalSocket* socket = nullptr) const;

 private:
  // The id of the user who started the currently active session.
  //
  // A value of -1 means there is no currently ongoing session.
  int m_sessionOwner = -1;
};

#endif  // DAEMONSESSION_H
