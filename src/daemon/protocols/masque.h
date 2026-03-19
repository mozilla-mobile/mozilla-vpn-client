/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MASQUE_H
#define MASQUE_H

#include "tunnel.h"

constexpr const char* MASQUE_INTERFACE = "mozmasque0";

class MasqueTunnel :  public Tunnel {
    Q_OBJECT
    public:
        MasqueTunnel(QObject* parent) : Tunnel(parent) {};
        Server::ProtocolType protocolType() const override { return Server::ProtocolType::Masque; }
        QString interfaceName() const override { return MASQUE_INTERFACE; };
        QJsonObject getStatus() const override;
        int checkHandshake() override;
        bool activate(const InterfaceConfig& config) override;
        bool switchServer(const InterfaceConfig& config) override;
};

#endif  // MASQUE_H