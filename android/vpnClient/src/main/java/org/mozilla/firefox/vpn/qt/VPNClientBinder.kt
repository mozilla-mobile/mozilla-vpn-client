/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn
import android.os.Binder
import android.os.Parcel
import org.mozilla.firefox.vpn.qt.VPNActivity
import android.util.Log

const val vpnPermissionRequired = 6
const val notificationPermissionRequired= 8

class VPNClientBinder() : Binder() {

    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        if (code == vpnPermissionRequired) {
            VPNActivity.getInstance().onPermissionRequest(code, data)
            return true
        }else if(code == notificationPermissionRequired){
            Log.e("VPNActivity", "ASKING FOR PERMISSIOOONS")
            VPNActivity.getInstance().onNotificationPermissionRequest()
        }

        val buffer = data.createByteArray()
        val stringData = buffer?.let { String(it) }
        VPNActivity.getInstance().onServiceMessage(code, stringData)
        return true
    }
}
