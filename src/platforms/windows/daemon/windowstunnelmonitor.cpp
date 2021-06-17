/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowstunnelmonitor.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"

#include <QDateTime>
#include <QScopeGuard>

#include <Windows.h>

constexpr uint32_t WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC = 2000;

/* The ring logger format used by the Wireguard DLL is as follows, assuming
 * no padding:
 *
 * struct {
 *   uint32_t magic;
 *   uint32_t index;
 *   struct {
 *     uint64_t timestamp;
 *     char message[512];
 *   } ring[2048];
 * };
 */

constexpr uint32_t RINGLOG_POLL_MSEC = 250;
constexpr uint32_t RINGLOG_MAGIC_HEADER = 0xbadbabe;
constexpr uint32_t RINGLOG_INDEX_OFFSET = 4;
constexpr uint32_t RINGLOG_HEADER_SIZE = 8;
constexpr uint32_t RINGLOG_MAX_ENTRIES = 2048;
constexpr uint32_t RINGLOG_MESSAGE_SIZE = 512;
constexpr uint32_t RINGLOG_TIMESTAMP_SIZE = 8;
constexpr uint32_t RINGLOG_FILE_SIZE =
    RINGLOG_HEADER_SIZE +
    ((RINGLOG_MESSAGE_SIZE + RINGLOG_TIMESTAMP_SIZE) * RINGLOG_MAX_ENTRIES);

namespace {
Logger logger(LOG_WINDOWS, "WindowsTunnelMonitor");
Logger logdll(LOG_WINDOWS, "tunnel.dll");
}  // namespace

WindowsTunnelMonitor::WindowsTunnelMonitor() {
  MVPN_COUNT_CTOR(WindowsTunnelMonitor);

  m_logEpochNsec = QDateTime::currentMSecsSinceEpoch() * 1000000;

  connect(&m_timer, &QTimer::timeout, this, &WindowsTunnelMonitor::timeout);
  connect(&m_logtimer, &QTimer::timeout, this,
          &WindowsTunnelMonitor::processLogs);
}

WindowsTunnelMonitor::~WindowsTunnelMonitor() {
  MVPN_COUNT_CTOR(WindowsTunnelMonitor);
}

void WindowsTunnelMonitor::resetLogs() {
  m_logEpochNsec = QDateTime::currentMSecsSinceEpoch() * 1000000;
}

void WindowsTunnelMonitor::start() {
  logger.log() << "Starting monitoring the tunnel service";
  m_timer.start(WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC);

  /* Open and map the tunnel log file. */
  QString logFileName = WindowsCommons::tunnelLogFile();
  if (logFileName.isNull()) {
    return;
  }
  m_logfile = new QFile(logFileName, this);
  m_logfile->open(QIODevice::ReadOnly);
  m_logindex = -1;
  m_logdata = m_logfile->map(0, RINGLOG_FILE_SIZE);
  if (!m_logdata) {
    return;
  }

  /* Check for a valid magic header */
  uint32_t magic;
  memcpy(&magic, m_logdata, 4);
  logger.log() << "Opening tunnel log file" << logFileName;
  if (magic != RINGLOG_MAGIC_HEADER) {
    logger.log() << "Unexpected magic header:" << QString::number(magic, 16);
    return;
  }

  m_logtimer.start(RINGLOG_POLL_MSEC);
}

void WindowsTunnelMonitor::stop() {
  logger.log() << "Stopping monitoring the tunnel service";
  m_timer.stop();
  m_logtimer.stop();

  if (m_logfile) {
    m_logfile->unmap(m_logdata);
    delete m_logfile;
    m_logfile = nullptr;
    m_logdata = nullptr;
  }
}

void WindowsTunnelMonitor::timeout() {
  SC_HANDLE scm;
  SC_HANDLE service;

  auto guard = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }

    if (scm) {
      CloseServiceHandle(scm);
    }
  });

  scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (!scm) {
    WindowsCommons::windowsLog("Failed to open SCManager");
    emit backendFailure();
    return;
  }

  // Let's see if we have to delete a previous instance.
  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (!service) {
    logger.log() << "The service doesn't exist";
    emit backendFailure();
    return;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    emit backendFailure();
    return;
  }

  if (status.dwCurrentState == SERVICE_RUNNING) {
    // The service is active
    return;
  }

  logger.log() << "The service is not active";
  emit backendFailure();
}

int WindowsTunnelMonitor::nextLogIndex() {
  if (!m_logdata) {
    return 0;
  }
  qint32 value;
  memcpy(&value, m_logdata + RINGLOG_INDEX_OFFSET, 4);
  return value % RINGLOG_MAX_ENTRIES;
}

void WindowsTunnelMonitor::processMessage(int index) {
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < RINGLOG_MAX_ENTRIES);
  size_t offset = index * (RINGLOG_TIMESTAMP_SIZE + RINGLOG_MESSAGE_SIZE);
  uchar* data = m_logdata + RINGLOG_HEADER_SIZE + offset;

  quint64 timestamp;
  memcpy(&timestamp, data, 8);
  if (timestamp <= m_logEpochNsec) {
    return;
  }

  QByteArray message((const char*)data + RINGLOG_TIMESTAMP_SIZE,
                     RINGLOG_MESSAGE_SIZE);
  logdll.log() << QString::fromUtf8(message);
}

void WindowsTunnelMonitor::processLogs() {
  /* On the first pass, scan all log messages. */
  if (m_logindex < 0) {
    m_logindex = nextLogIndex();
    processMessage(m_logindex);
    m_logindex = (m_logindex + 1) % RINGLOG_MAX_ENTRIES;
  }

  /* Report new messages. */
  while (m_logindex != nextLogIndex()) {
    processMessage(m_logindex);
    m_logindex = (m_logindex + 1) % RINGLOG_MAX_ENTRIES;
  }
}
