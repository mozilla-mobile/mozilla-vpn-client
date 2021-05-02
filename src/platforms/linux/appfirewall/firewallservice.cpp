/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QJsonDocument>
#include <QJsonObject>

#include "firewallservice.h"
#include "leakdetector.h"
#include "logger.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define VPN_EXCLUDE_CGROUP "/sys/fs/cgroup/net_cls/mozvpn.exclude"
constexpr const char* ROOT_CLASS_PROCS = "/sys/fs/cgroup/net_cls/cgroup.procs";
constexpr const char* VPN_EXCLUDE_PROCS = VPN_EXCLUDE_CGROUP "/cgroup.procs";
constexpr const char* VPN_EXCLUDE_CLASS_PATH =
    VPN_EXCLUDE_CGROUP "/net_cls.classid";
constexpr unsigned long VPN_EXCLUDE_CLASS_ID = 0x00110011;

namespace {
Logger logger(LOG_LINUX, "FirewallService");
}

FirewallService::FirewallService(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(FirewallService);
  m_adaptor = new FirewallAdaptor(this);

  int err;
  err = mkdir(VPN_EXCLUDE_CGROUP,
              S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  if ((err < 0) && (errno != EEXIST)) {
    logger.log() << "Failed to create VPN exclusion cgroup:" << strerror(errno);
    return;
  }
  if (err == 0) {
    FILE* fpclass = fopen(VPN_EXCLUDE_CLASS_PATH, "w");
    if (fpclass) {
      fprintf(fpclass, "%lu", VPN_EXCLUDE_CLASS_ID);
    }
    fclose(fpclass);
  }

  m_pidtracker = new PidTracker(this);
  connect(m_pidtracker, SIGNAL(pidForked(const QString&, int, int)), this,
          SLOT(pidForked(const QString&, int, int)));
  connect(m_pidtracker, SIGNAL(pidExited(const QString&, int)), this,
          SLOT(pidExited(const QString&, int)));
  connect(m_pidtracker, SIGNAL(terminated(const QString&, int)), this,
          SLOT(appTerminate(const QString&, int)));
}

FirewallService::~FirewallService() { MVPN_COUNT_DTOR(FirewallService); }

QString FirewallService::version() {
  logger.log() << "Version request";
  return PROTOCOL_VERSION;
}

QString FirewallService::status() {
  logger.log() << "Status request";
  QJsonObject json;
  json.insert("status", QJsonValue(true));
  return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

static inline void addToControlGroup(const char* cgprocs, int pid) {
  FILE* fp = fopen(cgprocs, "w");
  if (fp) {
    fprintf(fp, "%d", pid);
  }
  fclose(fp);
}

void FirewallService::excludeApp(const QStringList& names) {
  for (auto app = names.begin(); app != names.end(); app++) {
    logger.log() << "Adding" << *app << "to VPN exclusion group";
    if (m_excludedApps.contains(*app)) continue;
    m_excludedApps.append(*app);

    for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
      const ProcessGroup* group = m_pidtracker->group(*pid);
      if (group->name == *app) {
        addToControlGroup(VPN_EXCLUDE_PROCS, *pid);
      }
    }
  }
}

void FirewallService::includeApp(const QStringList& names) {
  for (auto app = names.begin(); app != names.end(); app++) {
    logger.log() << "Removing" << *app << "from VPN exclusion group";
    m_excludedApps.removeAll(*app);

    for (auto pid = m_pidtracker->begin(); pid != m_pidtracker->end(); pid++) {
      const ProcessGroup* group = m_pidtracker->group(*pid);
      if (group->name == *app) {
        addToControlGroup(ROOT_CLASS_PROCS, *pid);
      }
    }
  }
}

void FirewallService::pidForked(const QString& name, int parent, int child) {
  if (m_excludedApps.contains(name)) {
    addToControlGroup(VPN_EXCLUDE_PROCS, child);
  }
  logger.log() << "fork:" << name << "PID:" << parent << "->" << child;
}

void FirewallService::pidExited(const QString& name, int pid) {
  logger.log() << "exit:" << name << "PID:" << pid;
}

void FirewallService::appTerminate(const QString& name, int rootpid) {
  logger.log() << "terminate:" << name << "PID:" << rootpid;
}

void FirewallService::trackApp(const QString& name, int rootpid) {
  if (m_excludedApps.contains(name)) {
    addToControlGroup(VPN_EXCLUDE_PROCS, rootpid);
  }
  logger.log() << "tracking:" << name << "PID:" << rootpid;
  m_pidtracker->track(name, rootpid);
}