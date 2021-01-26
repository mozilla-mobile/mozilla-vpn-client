/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "wgquickprocess.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QProcess>
#include <QTextStream>
#include <QtGlobal>

namespace {
Logger logger(LOG_MACOS, "MacOSDaemon");
MacOSDaemon* s_daemon = nullptr;
}  // namespace

MacOSDaemon::MacOSDaemon() : Daemon(nullptr) {
  MVPN_COUNT_CTOR(MacOSDaemon);

  logger.log() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

MacOSDaemon::~MacOSDaemon() {
  MVPN_COUNT_DTOR(MacOSDaemon);

  logger.log() << "Daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
MacOSDaemon* MacOSDaemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

bool MacOSDaemon::activate(const Config& config) {
  if (!Daemon::activate(config)) {
    return false;
  }

  m_connectionDate = QDateTime::currentDateTime();
  return true;
}

void MacOSDaemon::status(QLocalSocket* socket) {
  logger.log() << "Status request";
  Q_ASSERT(socket);

  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", m_connected);

  if (m_connected) {
    uint64_t txBytes = 0;
    uint64_t rxBytes = 0;

    QDir appPath(QCoreApplication::applicationDirPath());
    appPath.cdUp();
    appPath.cd("Resources");
    appPath.cd("utils");
    QString wgPath = appPath.filePath("wg");

    QStringList arguments{"show", "all", "transfer"};
    logger.log() << "Start:" << wgPath << " - arguments:" << arguments;

    QProcess wgProcess;
    wgProcess.start(wgPath, arguments);

    if (!wgProcess.waitForFinished(-1)) {
      logger.log() << "Error occurred" << wgProcess.errorString();
    } else {
      QByteArray output = wgProcess.readAllStandardOutput();

      logger.log() << "wg-quick stdout:" << Qt::endl
                   << qUtf8Printable(output) << Qt::endl;
      logger.log() << "wg-quick stderr:" << Qt::endl
                   << qUtf8Printable(wgProcess.readAllStandardError())
                   << Qt::endl;

      QStringList lines = QString(output).split("\n");
      for (const QString& line : lines) {
        QStringList parts = line.split("\t");

        if (parts.length() == 4) {
          txBytes = parts[2].toLongLong();
          rxBytes = parts[3].toLongLong();
        }
      }
    }

    obj.insert("status", true);
    obj.insert("serverIpv4Gateway", m_lastConfig.m_serverIpv4Gateway);
    obj.insert("date", m_connectionDate.toString());

    obj.insert("txBytes", QJsonValue(double(txBytes)));
    obj.insert("rxBytes", QJsonValue(double(rxBytes)));
  }

  socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  socket->write("\n");
}

void MacOSDaemon::logs(QLocalSocket* socket) {
  logger.log() << "Log request";

  Q_ASSERT(socket);

  QJsonObject obj;
  obj.insert("type", "logs");
  obj.insert("logs", Daemon::logs().replace("\n", "|"));
  socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  socket->write("\n");
}

bool MacOSDaemon::run(Daemon::Op op, const Config& config) {
  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  return WgQuickProcess::run(
      op, config.m_privateKey, config.m_deviceIpv4Address,
      config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
      config.m_serverIpv6Gateway, config.m_serverPublicKey,
      config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
      addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled);
}
