package org.mozilla.firefox.vpn

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import android.net.wifi.ScanResult
import android.net.wifi.WifiInfo
import android.net.wifi.WifiManager
import android.util.Log
import org.mozilla.firefox.vpn.VPNService


const val TYPE_WIFI = 1
const val TYPE_MOBILE = 2
const val TYPE_NOT_CONNECTED = 0

class NetworkChangeReceiver : BroadcastReceiver() {
    private val tag ="NetworkChangeReceiver"

    override fun onReceive(context: Context?, intent: Intent) {

        if ("android.net.conn.CONNECTIVITY_CHANGE" != intent.action &&
            "android.net.wifi.WIFI_STATE_CHANGED" != intent.action &&
            intent.action != WifiManager.SCAN_RESULTS_AVAILABLE_ACTION) {
            // Not a connectivity change ?
            return;
        }
        val status: Int? = context?.let { getConnectivityStatus(it) }
        if(status != TYPE_WIFI){
            // We only care for unsecured wifi connections.
            return;
        }
        val network = getCurrentWifi(context)
            ?:
            return

        //get capabilities of current connection
        val capabilities = network.capabilities
        Log.d(tag, network.SSID + " capabilities : " + capabilities)
        if (capabilities.contains("WPA2") || capabilities.contains("WPA")) {
            // All good
            return;
        }
        // Probably an insecure network :(
        try {
            insecureNetworkDetected(network.SSID, network.BSSID)
        }catch (e: Error){
        }
    }
    // Defined in AndroidNetworkWatcher.cpp
    external fun insecureNetworkDetected(ssid: String, bssid: String);

    private fun getConnectivityStatus(context: Context): Int {
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val activeNetwork = cm.activeNetworkInfo
        if (null != activeNetwork) {
            if (activeNetwork.type == ConnectivityManager.TYPE_WIFI) return TYPE_WIFI
            if (activeNetwork.type == ConnectivityManager.TYPE_MOBILE) return TYPE_MOBILE
        }
        return TYPE_NOT_CONNECTED
    }

    private fun getCurrentWifi(context: Context): ScanResult? {
        val wifiManager =
            context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val networkList: List<ScanResult> = wifiManager.scanResults
        if(networkList.isEmpty()){
            // We'll come back once the results are in.
            wifiManager.startScan();
            return null;
        }
        //get current connected SSID for comparison to ScanResult
        val wi: WifiInfo = wifiManager.connectionInfo
        val currentSSID = wi.ssid
        for (network in networkList) {
            //check if current connected SSID
            if (currentSSID == network.SSID) {
                    return network;
            }
        }

        return null
    }


    companion object {
        var s_instance: NetworkChangeReceiver? = null
        @JvmStatic
        fun registerIntentFilter(c: Context) {
            if(s_instance == null){
                s_instance = NetworkChangeReceiver()
            }
            val filter = IntentFilter()
            filter.addAction("android.net.conn.CONNECTIVITY_CHANGE")
            filter.addAction("android.net.wifi.WIFI_STATE_CHANGED")
            filter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)
            c.applicationContext.registerReceiver(s_instance, filter)
        }
    }

}