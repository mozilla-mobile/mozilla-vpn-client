/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKMASQUE_H
#define MOCKMASQUE_H

#include "daemon/protocols/masque.h"

class MockMasqueTunnel : public MasqueTunnel {
    Q_OBJECT
        public:
        MockMasqueTunnel(QObject* parent) : MasqueTunnel(parent) { };
        bool addInterface(const InterfaceConfig& config) override { return true; };
        bool deleteInterface() override { return true; };
        bool switchServer(const InterfaceConfig& config) override { return true; };
        bool activate(const InterfaceConfig& config) override { return true; };
        bool supportSplitTunnel() override { return false; }
        void excludeApp(const QString& app) {};
        void resetApp(const QString &app) {};
        void resetAllApps() const {};
};

#endif  // MOCKMASQUE_H