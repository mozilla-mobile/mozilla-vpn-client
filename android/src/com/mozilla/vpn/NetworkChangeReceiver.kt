/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import com.mozilla.vpn.CaptivePortalDetector
import com.wireguard.android.backend.Tunnel

const val TYPE_WIFI = 1
const val TYPE_MOBILE = 2
const val TYPE_NOT_CONNECTED = 0

class NetworkChangeReceiver(c: Context) : BroadcastReceiver() {
    private val context=c
    private val tag = "NetworkChangeReceiver"
    private var active=false;

    override fun onReceive(context: Context?, intent: Intent) {
        if(!active){
            return;
        }

        if ("android.net.conn.CONNECTIVITY_CHANGE" != intent.action &&
            "android.net.wifi.WIFI_STATE_CHANGED" != intent.action
        ) {
            // Not a connectivity change ?
            return
        }
        // continue if the Current Connection is WIFI
        val status: Int? = context?.let { getConnectivityStatus(it) }
        if (status != TYPE_WIFI) {
            return
        }
        // Only check for a Captive Portal if the vpn is connected
        VPNService.instance?.let {
            if (it.state == Tunnel.State.UP) {
                CaptivePortalDetector.get(context).detectPortal()
            }
        }
    }

    private fun getConnectivityStatus(context: Context): Int {
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val activeNetwork = cm.activeNetworkInfo
        if (null != activeNetwork) {
            if (activeNetwork.type == ConnectivityManager.TYPE_WIFI) return TYPE_WIFI
            if (activeNetwork.type == ConnectivityManager.TYPE_MOBILE) return TYPE_MOBILE
        }
        return TYPE_NOT_CONNECTED
    }

    fun activate(){
        val filter = IntentFilter()
        filter.addAction("android.net.conn.CONNECTIVITY_CHANGE")
        filter.addAction("android.net.wifi.WIFI_STATE_CHANGED")
        context.applicationContext.registerReceiver(s_instance, filter)
        active=true
    }
    fun deactivate(){
        active=false
    }

    companion object {
        var s_instance: NetworkChangeReceiver? = null
        @JvmStatic
        fun get(c: Context): NetworkChangeReceiver {
            if (s_instance == null) {
                s_instance = NetworkChangeReceiver(c)
            }
            return s_instance as NetworkChangeReceiver
        }
    }
}
