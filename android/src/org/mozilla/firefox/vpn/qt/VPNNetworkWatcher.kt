/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.Build
import android.provider.Settings

@Suppress("unused") // JNI-Called AndroidTransportWatcher.cpp
object VPNNetworkWatcher {

    object TransportTypes {
        const val Unknown = 0
        const val Ethernet = 1
        const val WiFi = 2
        const val Cellular = 3
        const val Other = 4 // I.e USB thethering
        const val None = 5 // I.e Airplane Mode or no active network device
    }

    @JvmStatic
    @Suppress("unused") // JNI-Called
    fun getTransportType(con: Context): Int {
        val airplaneMode = Settings.Global.getInt(con.contentResolver, Settings.Global.AIRPLANE_MODE_ON, 0)
        if (airplaneMode != 0) {
            return TransportTypes.None
        }
        val connectivityManager: ConnectivityManager = con.getSystemService(Context.CONNECTIVITY_SERVICE) as(ConnectivityManager)
        val network = connectivityManager.activeNetwork

        val capabilities = connectivityManager.getNetworkCapabilities(network)
            ?: return TransportTypes.Unknown
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.Q) {
            return TransportTypes.Unknown
        }
        if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_BLUETOOTH)) {
            return TransportTypes.Other
        }
        if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET)) {
            return TransportTypes.Ethernet
        }
        if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
            return TransportTypes.WiFi
        }
        if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
            return TransportTypes.Cellular
        }
        return TransportTypes.Unknown
    }
}
