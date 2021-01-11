/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import com.wireguard.android.backend.Tunnel
import org.mozilla.firefox.vpn.VPNService
class VPNTunnel: Tunnel{
    val mName:String;
    val mService:VPNService;

    constructor(name:String, s: VPNService ){
        this.mName = name;
        this.mService =s;
    }

    override fun getName(): String {
        return mName;
    }
    override fun onStateChange(newState: Tunnel.State) {
        mService.onTunnelStateChange(newState);
        return;
    }
}