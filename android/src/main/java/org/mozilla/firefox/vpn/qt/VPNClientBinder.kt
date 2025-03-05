/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn
import android.os.Parcel
import org.mozilla.firefox.qt.common.CoreBinder
import org.mozilla.firefox.vpn.qt.VPNActivity

class VPNClientBinder() : CoreBinder() {

    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        when (code) {
            EVENTS.permissionRequired -> {
                VPNActivity.getInstance().onPermissionRequest(code, data)
                return true
            }
            EVENTS.requestNotificationPermission -> {
                VPNActivity.getInstance().onNotificationPermissionRequest()
                return true
            }
            else -> {
                val buffer = data.createByteArray()
                val stringData = buffer?.let { String(it) }
                VPNActivity.getInstance().onServiceMessage(code, stringData)
                return true
            }
        }
    }
}
