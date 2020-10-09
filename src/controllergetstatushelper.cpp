#include "controllergetstatushelper.h"

ControllerGetStatusHelper::ControllerGetStatusHelper(
    QObject *parent,
    std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
        &&callback)
    : QObject(parent), m_callback(std::move(callback))
{}

void ControllerGetStatusHelper::statusUpdated(const QString &serverIpv4Gateway,
                                              uint64_t txBytes,
                                              uint64_t rxBytes)
{
    deleteLater();
    m_callback(serverIpv4Gateway, txBytes, rxBytes);
}
