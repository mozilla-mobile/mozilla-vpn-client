/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"

void Controller::updateRequired()
{
}

void Controller::getStatus(std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)> &&a_callback) {
    std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
        callback = std::move(a_callback);
    callback("127.0.0.1", 0, 0);
}
