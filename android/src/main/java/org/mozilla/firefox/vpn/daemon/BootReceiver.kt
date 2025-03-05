/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import org.mozilla.firefox.qt.common.Prefs

class BootReceiver : BroadcastReceiver() {
    private val TAG = "BootReceiver"

    override fun onReceive(context: Context, arg1: Intent) {
        Log.init(context)
        if (!Prefs.get(context).getBoolean(START_ON_BOOT, false)) {
            Log.i(TAG, "This device did not enable start on boot - exit")
            return
        }
        Log.i(TAG, "This device did enable start on boot - try to start")
        val intent = Intent(context, VPNService::class.java)
        intent.putExtra("startOnBoot", true)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent)
        } else {
            context.startService(intent)
        }
        Log.i(TAG, "Queued VPNService start")
    }
    companion object {
        const val START_ON_BOOT = "startOnBoot"
    }
}
