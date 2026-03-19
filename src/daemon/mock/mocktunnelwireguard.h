/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKWIREGUARD_H
#define MOCKWIREGUARD_H

#include "daemon/protocols/wireguard.h"
#include "wireguardutilsmock.h"



class MockWireGuardTunnel : public WireGuardTunnel {
    Q_OBJECT
        public:
        MockWireGuardTunnel(QObject* parent) : WireGuardTunnel(parent) {   m_wgutils = new WireguardUtilsMock(this);
};
        QString interfaceName() const override { return wgutils()->interfaceName(); };
        bool interfaceExists() override { return wgutils()->interfaceExists(); };
        QJsonObject getStatus() const override { return QJsonObject(); };
        int checkHandshake() override { return 0; };
        bool addInterface(const InterfaceConfig& config) override { return true; };
        bool deleteInterface() override { return true; };
        bool switchServer(const InterfaceConfig& config) override { return true; };
        bool activate(const InterfaceConfig& config) override { return true; };
        bool supportIPUtils() const { return false; }
        bool supportSplitTunnel() override { return false; }
        void excludeApp(const QString& app) override {};
        void resetApp(const QString &app) override {};
        void resetAllApps() override {};

        private:
           WireguardUtils* wgutils() const override { return m_wgutils; }
           WireguardUtilsMock* m_wgutils = nullptr;

};

#endif  // MOCKWIREGUARD_H