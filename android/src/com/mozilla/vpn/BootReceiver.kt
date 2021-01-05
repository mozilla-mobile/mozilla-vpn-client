/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import android.util.Log
import org.mozilla.firefox.vpn.VPNService

class BootReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, arg1: Intent) {
        if(!canEnableVPNOnBoot()){
            return;
        }
        val prefs = context.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE);
        val startOnBoot =  prefs.getBoolean("startOnBoot",false)
        if(!startOnBoot){
            return;
        }
        val intent = Intent(context, VPNService::class.java)
        intent.putExtra("startOnBoot",true)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent)
        } else {
            context.startService(intent)
        }
        Log.i("BootReceiver", "Started Service")
    }

    companion object{
        /**
         * Only devices below N allow us to enable the VPN after the OnBootIntent
         * Devices higher then that should refer to the Always On VPN option in settings
         */
        fun canEnableVPNOnBoot(): Boolean {
            return (Build.VERSION.SDK_INT < Build.VERSION_CODES.N)
        }
    }
}