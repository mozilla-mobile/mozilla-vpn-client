/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import com.wireguard.android.backend.Tunnel
import org.mozilla.firefox.vpn.VPNService
import org.mozilla.firefox.vpn.VPNServiceBinder

class VPNTunnel : Tunnel {
    val mName: String;
    val mBinder: VPNServiceBinder;
    var mState = Tunnel.State.DOWN;

    constructor(name: String, m: VPNServiceBinder) {
        this.mName = name;
        this.mBinder = m;
    }

    override fun getName(): String {
        return mName;
    }

    override fun onStateChange(newState: Tunnel.State) {
        if (mState != newState) {
            mState = newState;
            if (mState == Tunnel.State.UP) {
                mBinder.dispatchEvent(VPNServiceBinder.EVENTS.connected, "")
            } else {
                mBinder.dispatchEvent(VPNServiceBinder.EVENTS.disconnected, "")
            }
        }
    }
}