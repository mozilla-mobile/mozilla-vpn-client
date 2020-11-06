/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H

#include "controller.h"

#include <QObject>

#include <functional>

class Keys;
class Device;
class Server;
class QDateTime;
class IPAddressRange;

class ControllerImpl : public QObject
{
    Q_OBJECT

public:
    ControllerImpl() = default;

    virtual ~ControllerImpl() = default;

    // This method is called to initialize the controller. The initialization
    // is completed when the signal "initialized" is emitted.
    virtual void initialize(const Device *device, const Keys *keys) = 0;

    // This method is called when the VPN client needs to activate the VPN
    // tunnel. It's called only at the end of the initialization process.  When
    // this method is called, the VPN client is in "connecting" state.  This
    // state terminates when the "connected" (or the "disconnected") signal is
    // received.
    virtual void activate(const Server &server,
                          const Device *device,
                          const Keys *keys,
                          const QList<IPAddressRange> &allowedIPAddressRanges,
                          bool forSwitching)
        = 0;

    // This method terminates the VPN tunnel. The VPN client is in
    // "disconnecting" state until the "disconnected" signal is received.
    virtual void deactivate(bool forSwitching) = 0;

    // This method is used to retrieve the VPN tunnel status (mainly the number
    // of bytes sent and received). It's called always when the VPN tunnel is
    // active.
    virtual void checkStatus() = 0;

    // This method is used to retrieve the logs from the backend service. Use
    // the callback to report logs when available.
    virtual void getBackendLogs(std::function<void(const QString &logs)> &&callback) = 0;

signals:
    // This signal is emitted when the controller is initialized. Note that the
    // VPN tunnel can be already active. In this case, "state" should be set to
    // StateOn and the "connectionDate" should be set to the activation date if
    // known.
    // If "status" is set to false, the backend service is considered unavailable.
    void initialized(bool status, Controller::State state, const QDateTime &connectionDate);

    // These 2 signals can be dispatched at any time.
    void connected();
    void disconnected();

    // This method should be emitted after a checkStatus() call.
    // "serverIpv4Gateway" is the current VPN tunnel gateway. "txBytes" and
    // "rxBytes" contain the number of transmitted and received bytes since the
    // last statusUpdated signal.
    void statusUpdated(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes);
};

#endif // CONTROLLERIMPL_H
