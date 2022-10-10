/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */ 

const vpn = require('./helper.js');
 
const connectionChangeNotification = async () => {
    return await vpn.setSetting('connection-change-notification', 'true');
}

const serverSwitchNotification = async () => {    
    return await vpn.setSetting('server-switch-notification', 'true');
}
 

module.exports = {
    actions: {
        settings: {
            connectionChangeNotification,
            serverSwitchNotification,
        }
    }
}