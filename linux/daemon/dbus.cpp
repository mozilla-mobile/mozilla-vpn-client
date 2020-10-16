/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbus.h"
#include "../../src/logger.h"
#include "../../src/loghandler.h"
#include "dbus_adaptor.h"
#include "polkithelper.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"

#if defined(__cplusplus)
}
#endif

namespace {
Logger logger("DBus");
}

DBus::DBus(QObject *parent) : QObject(parent) {}

void DBus::setAdaptor(DbusAdaptor* adaptor)
{
    Q_ASSERT(!m_adaptor);
    m_adaptor = adaptor;
}

bool DBus::checkInterface()
{
    logger.log() << "Checking interface";

    wg_device *device = nullptr;
    if (wg_get_device(&device, WG_INTERFACE) == 0) {
        logger.log() << "Device already exists. Let's remove it.";
        wg_free_device(device);

        if (wg_del_device(WG_INTERFACE) != 0) {
            logger.log() << "Failed to remove the device.";
            return false;
        }
    }

    return true;
}

QString DBus::version()
{
    logger.log() << "Version request";
    return APP_VERSION;
}

bool DBus::activate(const QString &jsonConfig)
{
    logger.log() << "Activate";

    if (!PolkitHelper::instance()->checkAuthorization("org.mozilla.vpn.activate")) {
        logger.log() << "Polkit rejected";
        return false;
    }

    if (m_connected) {
        logger.log() << "Already connected";
        return false;
    }

    m_connected = true;

    QJsonDocument json = QJsonDocument::fromJson(jsonConfig.toLocal8Bit());
    if (!json.isObject()) {
        logger.log() << "Invalid input";
        return false;
    }

    QJsonObject obj = json.object();

#define GETVALUESTR(name, where) \
    if (!obj.contains(name)) { \
        logger.log() << name << " missing in the jsonConfig input"; \
        return false; \
    } \
    { \
        QJsonValue value = obj.take(name); \
        if (!value.isString()) { \
            logger.log() << name << " is not a string"; \
            return false; \
        } \
        where = value.toString(); \
    }

    GETVALUESTR("privateKey", m_lastPrivateKey);
    GETVALUESTR("deviceIpv4Address", m_lastDeviceIpv4Address);
    GETVALUESTR("deviceIpv6Address", m_lastDeviceIpv6Address);
    GETVALUESTR("serverIpv4Gateway", m_lastServerIpv4Gateway);
    GETVALUESTR("serverIpv6Gateway", m_lastServerIpv6Gateway);
    GETVALUESTR("serverPublicKey", m_lastServerPublicKey);
    GETVALUESTR("serverIpv4AddrIn", m_lastServerIpv4AddrIn);
    GETVALUESTR("serverIpv6AddrIn", m_lastServerIpv6AddrIn);

#undef GETVALUESTR

#define GETVALUEINT(name, where) \
    if (!obj.contains(name)) { \
        logger.log() << name << " missing in the jsoConfig input"; \
        return false; \
    } \
    { \
        QJsonValue value = obj.take(name); \
        if (!value.isDouble()) { \
            logger.log() << name << " is not a number"; \
            return false; \
        } \
        where = value.toInt(); \
    }

    GETVALUEINT("serverPort", m_lastServerPort);

#undef GETVALUEINT

#define GETVALUEBOOL(name, where) \
    if (!obj.contains(name)) { \
        logger.log() << name << " missing in the jsoConfig input"; \
        return false; \
    } \
    { \
        QJsonValue value = obj.take(name); \
        if (!value.isBool()) { \
            logger.log() << name << " is not a boolean"; \
            return false; \
        } \
        where = value.toBool(); \
    }

    GETVALUEBOOL("ipv6Enabled", m_lastIpv6Enabled);
    GETVALUEBOOL("localNetworkAccess", m_lastLocalNetworkAccess);

#undef GETVALUEBOOL

    bool status = runWgQuick(WgQuickProcess::Up,
                             m_lastPrivateKey,
                             m_lastDeviceIpv4Address,
                             m_lastDeviceIpv6Address,
                             m_lastServerIpv4Gateway,
                             m_lastServerIpv6Gateway,
                             m_lastServerPublicKey,
                             m_lastServerIpv4AddrIn,
                             m_lastServerIpv6AddrIn,
                             m_lastServerPort,
                             m_lastIpv6Enabled,
                             m_lastLocalNetworkAccess);

    logger.log() << "Status:" << status;

    if (status) {
        emit m_adaptor->connected();
    }

    return status;
}

bool DBus::deactivate()
{
    logger.log() << "Deactivate";

    if (!m_connected) {
        logger.log() << "Already disconnected";
        return true;
    }

    if (!PolkitHelper::instance()->checkAuthorization("org.mozilla.vpn.deactivate")) {
        logger.log() << "Polkit rejected";
        return false;
    }

    m_connected = false;

    bool status = runWgQuick(WgQuickProcess::Down,
                             m_lastPrivateKey,
                             m_lastDeviceIpv4Address,
                             m_lastDeviceIpv6Address,
                             m_lastServerIpv4Gateway,
                             m_lastServerIpv6Gateway,
                             m_lastServerPublicKey,
                             m_lastServerIpv4AddrIn,
                             m_lastServerIpv6AddrIn,
                             m_lastServerPort,
                             m_lastIpv6Enabled,
                             m_lastLocalNetworkAccess);

    logger.log() << "Status:" << status;

    if (status) {
        emit m_adaptor->disconnected();
    }

    return status;
}

QString DBus::status()
{
    logger.log() << "Status request";

    QJsonObject json;

    wg_device *device = nullptr;
    if (wg_get_device(&device, WG_INTERFACE) != 0) {
        logger.log() << "Unable to get device";
        json.insert("status", QJsonValue(false));
        return QJsonDocument(json).toJson();
    }

    uint64_t txBytes = 0;
    uint64_t rxBytes = 0;

    wg_peer *peer;
    wg_for_each_peer(device, peer)
    {
        txBytes += peer->tx_bytes;
        rxBytes += peer->rx_bytes;
    }

    wg_free_device(device);

    json.insert("status", QJsonValue(true));
    json.insert("serverIpv4Gateway", QJsonValue(m_lastServerIpv4Gateway));
    json.insert("txBytes", QJsonValue(double(txBytes)));
    json.insert("rxBytes", QJsonValue(double(rxBytes)));

    return QJsonDocument(json).toJson();
}

QString DBus::logs()
{
    logger.log() << "Log request";

    QString output;
    QTextStream out(&output);

    LogHandler *logHandler = LogHandler::instance();
    for (QVector<LogHandler::Log>::ConstIterator i = logHandler->logs().begin(); i != logHandler->logs().end();
         ++i) {
        logHandler->prettyOutput(out, *i);
    }

    return output;
}

bool DBus::runWgQuick(WgQuickProcess::Op op,
                      const QString &privateKey,
                      const QString &deviceIpv4Address,
                      const QString &deviceIpv6Address,
                      const QString &serverIpv4Gateway,
                      const QString &serverIpv6Gateway,
                      const QString &serverPublicKey,
                      const QString &serverIpv4AddrIn,
                      const QString &serverIpv6AddrIn,
                      int serverPort,
                      bool ipv6Enabled,
                      bool localNetworkAccess)
{
    WgQuickProcess *wgQuick = new WgQuickProcess(op);

    wgQuick->run(privateKey,
                 deviceIpv4Address,
                 deviceIpv6Address,
                 serverIpv4Gateway,
                 serverIpv6Gateway,
                 serverPublicKey,
                 serverIpv4AddrIn,
                 serverIpv6AddrIn,
                 serverPort,
                 ipv6Enabled,
                 localNetworkAccess);

    enum Result {
        Pending,
        Success,
        Fail,
    };

    Result result = Pending;
    connect(wgQuick, &WgQuickProcess::failed, [&] { result = Fail; });
    connect(wgQuick, &WgQuickProcess::succeeded, [&] { result = Success; });

    while (result == Pending) {
        QCoreApplication::processEvents();
    }

    return result == Success;
}

