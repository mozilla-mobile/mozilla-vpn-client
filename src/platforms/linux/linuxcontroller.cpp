#include "linuxcontroller.h"
#include "device.h"
#include "dbus.h"
#include "errorhandler.h"
#include "keys.h"
#include "mozillavpn.h"
#include "server.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QString>

void LinuxController::initialize(const Device *device, const Keys *keys)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);

    DBus *dbus = new DBus(this);
    connect(dbus, &DBus::statusReceived, [this](const QString &status) {
        qDebug() << "Status:" << status;

        QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
        Q_ASSERT(json.isObject());

        QJsonObject obj = json.object();
        Q_ASSERT(obj.contains("status"));
        QJsonValue statusValue = obj.take("status");
        Q_ASSERT(statusValue.isBool());

        emit initialized(true,
                         statusValue.toBool() ? Controller::StateOn : Controller::StateOff,
                         QDateTime::currentDateTime());
    });

    connect(dbus, &DBus::failed, [this]() {
        emit initialized(false, Controller::StateOff, QDateTime());
    });

    dbus->status();
}

void LinuxController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_ASSERT(device);
    Q_ASSERT(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "LinuxController activated";

    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, [this]() {
        MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
        emit disconnected();
    });

    connect(dbus, &DBus::succeeded, this, &LinuxController::connected);

    dbus->activate(server, device, keys);
}

void LinuxController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    qDebug() << "LinuxController deactivated";

    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, this, &LinuxController::disconnected);
    connect(dbus, &DBus::succeeded, this, &LinuxController::disconnected);

    dbus->deactivate();
}

void LinuxController::checkStatus()
{
    qDebug() << "Check status";

    DBus *dbus = new DBus(this);
    connect(dbus, &DBus::statusReceived, [this](const QString &status) {
        qDebug() << "Status:" << status;

        QJsonDocument json = QJsonDocument::fromJson(status.toLocal8Bit());
        Q_ASSERT(json.isObject());

        QJsonObject obj = json.object();
        Q_ASSERT(obj.contains("status"));
        QJsonValue statusValue = obj.take("status");
        Q_ASSERT(statusValue.isBool());

        if (!statusValue.toBool()) {
            qDebug() << "Unable to retrieve the status from the interface.";
            return;
        }

        Q_ASSERT(obj.contains("txBytes"));
        QJsonValue txBytes = obj.take("txBytes");
        Q_ASSERT(txBytes.isDouble());

        Q_ASSERT(obj.contains("rxBytes"));
        QJsonValue rxBytes = obj.take("rxBytes");
        Q_ASSERT(rxBytes.isDouble());

        emit statusUpdated(txBytes.toDouble(), rxBytes.toDouble());
    });

    dbus->status();
}
