/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKMASQUESERVERS_H
#define TASKMASQUESERVERS_H

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>

#include "errorhandler.h"
#include "task.h"

class QHostInfo;

// Fetches the MASQUE server list from the Remote Settings "vpn-serverlist"
// changeset, resolves each server's hostname to an IP address, and merges the
// result into the ServerCountryModel.
class TaskMasqueServers final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskMasqueServers)

 public:
  explicit TaskMasqueServers(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskMasqueServers();

  void run() override;

 private slots:
  void dnsLookupCompleted(const QHostInfo& hostInfo);

 private:
  // Kick off a DNS lookup for every unique hostname in the changeset.
  void resolveHostnames(const QByteArray& changeset);
  // Inject the resolved IPs back into the changeset and hand it to the model.
  void finalize();

  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;

  QJsonObject m_root;
  QHash<QString, QString> m_ipv4;
  QHash<QString, QString> m_ipv6;
  QList<int> m_lookupIds;
  int m_pendingLookups = 0;
};

#endif  // TASKMASQUESERVERS_H
