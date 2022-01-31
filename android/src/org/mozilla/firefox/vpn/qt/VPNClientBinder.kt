/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn
import android.os.Binder
import android.os.Parcel
import com.wireguard.config.*
import org.mozilla.firefox.vpn.qt.VPNActivity

class VPNClientBinder() : Binder() {

    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        val buffer = data.createByteArray()
        val stringData = buffer?.let { String(it) }
        VPNActivity.getInstance().onServiceMessage(code, stringData)
        return true
    }
}
