/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import com.wireguard.android.backend.Tunnel

class VPNTunnel: Tunnel{
    val mName:String;
    constructor(name:String){
        this.mName = name;
    }

    override fun getName(): String {
        return mName;
    }
    override fun onStateChange(newState: Tunnel.State) {
        return;
    }
}