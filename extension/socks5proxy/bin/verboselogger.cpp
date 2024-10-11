/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "verboselogger.h"

#include <QDateTime>
#include <QDir>
#include <QLoggingCategory>
#include <QFile>

#include "socks5.h"

// 4MB of log data ought to be enough for anyone.
constexpr const qsizetype LOGFILE_MAX_SIZE = 4 * 1024 * 1024;

// static
QString VerboseLogger::bytesToString(qint64 bytes) {
  if (bytes < 1024) {
    return QString("%1b").arg(bytes);
  }

  if (bytes < 1024 * 1024) {
    return QString("%1Kb").arg(bytes / 1024);
  }

  if (bytes < 1024 * 1024 * 1024) {
    return QString("%1Mb").arg(bytes / (1024 * 1024));
  }

  return QString("%1Gb").arg(bytes / (1024 * 1024 * 1024));
}

VerboseLogger::VerboseLogger(Socks5* socks5)
    : QObject(socks5), m_socks(socks5) {
  connect(&m_timer, &QTimer::timeout, this, &VerboseLogger::tick);
  m_timer.setSingleShot(false);
  m_timer.start(1000);

  connect(socks5, &Socks5::connectionsChanged, this,
          &VerboseLogger::printStatus);

  connect(socks5, &Socks5::incomingConnection, this,
          [this](Socks5Connection* conn) {
            connect(conn, &Socks5Connection::dataSentReceived, this,
                    &VerboseLogger::dataSentReceived);
            connect(conn, &Socks5Connection::stateChanged, this,
                    &VerboseLogger::connectionStateChanged);

            m_events.append(
                Event{conn->clientName(), QDateTime::currentMSecsSinceEpoch()});
            printStatus();
          });

  // Store our singleton reference.
  s_instance = this;

  // Install a custom log handler that plays nicely with the status.
  QLoggingCategory::defaultCategory()->setEnabled(QtMsgType::QtDebugMsg, true);
  qInstallMessageHandler(logHandler);
}

VerboseLogger::~VerboseLogger() {
  qInstallMessageHandler(nullptr);
  s_instance = nullptr;
}

void VerboseLogger::tick() {
  // Update the boxcar average.
  m_tx_bytes.advance();
  m_rx_bytes.advance();

  // Drop entries from the event queue.
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  QMutableListIterator<Event> i(m_events);
  while (i.hasNext()) {
    if ((now - i.next().m_when) > 1000) {
      i.remove();
    }
  }

  // Update the status.
  printStatus();

  // Handle logfile rotation.
  QMutexLocker lock(&m_logFileMutex);
  if (m_logFileDevice && (m_logFileDevice->size() > LOGFILE_MAX_SIZE)) {
    // Truncate the file.
    m_logFileDevice->seek(0);
    m_logFileDevice->resize(0);
  }
}

VerboseLogger* VerboseLogger::s_instance = nullptr;

void VerboseLogger::printStatus() {
  QString output;
  {
    QTextStream out(&output);
    out << "Connections: " << m_socks->connections();

    QStringList addresses;
    for (const Event& event : m_events) {
      if (!event.m_newConnection.isEmpty() &&
          !addresses.contains(event.m_newConnection)) {
        addresses.append(event.m_newConnection);
      }
    }
    out << " [" << addresses.join(", ") << "]";
    out << " Up: " << bytesToString(m_tx_bytes.average()) << "/s";
    out << " Down: " << bytesToString(m_rx_bytes.average()) << "/s";
  }

  output.truncate(80);
  while (output.length() < 80) output.append(' ');
  QTextStream out(stdout);
  out << output << '\r';

  m_lastStatus = output;
}

void VerboseLogger::dataSentReceived(qint64 sent, qint64 received) {
  m_tx_bytes.addSample(sent);
  m_rx_bytes.addSample(received);
}

void VerboseLogger::connectionStateChanged() {
  Socks5Connection* conn = qobject_cast<Socks5Connection*>(QObject::sender());
  if (conn->state() == Socks5Connection::Proxy) {
    auto msg = qDebug() << "Connecting" << conn->clientName() << "to";
    for (const QString& hostname : conn->dnsLookupStack()) {
      msg << hostname << "->";
    }
    msg << conn->destAddress().toString();
  }
}

// static and recursive!
bool VerboseLogger::makeLogDir(const QDir& dir) {
  if (dir.exists()) {
    return true;
  }
  // Recursively make the parent.
  QDir parent(dir);
  if (!parent.cdUp()) {
    return false;
  }
  if (!makeLogDir(parent)) {
    return false;
  }
  // Make ourself.
  return parent.mkdir(dir.dirName());
}

void VerboseLogger::setLogfile(const QString& logfile) {
  qInfo() << "Logging to:" << logfile;
  if (!makeLogDir(QFileInfo(logfile).dir())) {
    qInfo() << "Failed to create directory";
    return;
  }
  m_logFileName = logfile;

  QMutexLocker lock(&m_logFileMutex);

  // Close the log file, if any.
  if (m_logFileDevice) {
    m_logFileDevice->flush();
    m_logFileDevice->close();
    m_logFileDevice->deleteLater();
    m_logFileDevice = nullptr;
  }

  // Open the new log file, if any.
  if (!m_logFileName.isEmpty()) {
    m_logFileDevice = new QFile(m_logFileName, this);
    const auto mode = QIODeviceBase::WriteOnly | QIODeviceBase::Text |
                      QIODeviceBase::Append;
    const auto perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                       QFileDevice::ReadGroup;
    m_logFileDevice->open(mode, perms);
  }
}

void VerboseLogger::logfileHandler(
    QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
  QMutexLocker lock(&m_logFileMutex);
  if (m_logFileDevice) {
    // Write the line into the logfile.
    m_logFileDevice->write(msg.toUtf8() + '\n');
    m_logFileDevice->flush();
  }
}

// static
void VerboseLogger::logHandler(QtMsgType type, const QMessageLogContext& ctx,
                               const QString& msg) {
  QTextStream out(stdout);
  if (!s_instance) {
    // No logger? Just dump everything to console.
    out << msg << "\n";
    return;
  }

  // A message logger that plays nicely with the status output.
  // Clears the current line - prints the log message - reprints the status.
  out << QString(80, ' ') << '\r';
  out << msg << "\r\n";
  out << s_instance->m_lastStatus << '\r';

  // Handle logging to file.
  s_instance->logfileHandler(type, ctx, msg);
}
