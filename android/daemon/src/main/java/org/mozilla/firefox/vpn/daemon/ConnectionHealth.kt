/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.Context
import android.net.*
import android.os.Build
import android.os.CountDownTimer
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class ConnectionHealth(service: VPNService) {
    private val TAG = "DaemonConnectionHealth"
    private val mService: VPNService = service
    private val PING_TIMEOUT = 3000 // ms

    private var mEndPoint: String = ""
    private var mDNS: String = ""
    private var mGateway: String = ""
    private var mAltEndpoint: String = ""
    private var mResetUsed = false

    var mActive = false
    var mVPNNetwork: Network? = null
    var mWorker: ExecutorService = Executors.newSingleThreadExecutor()

    fun start(endpoint: String, gateway: String, dns: String, altEndpoint: String) {
        mEndPoint = endpoint
        mGateway = gateway
        mDNS = dns
        mAltEndpoint = altEndpoint
        if (mActive) {
            return
        }
        mResetUsed = false
        val mConnectivityManager = mService.getSystemService(Context.CONNECTIVITY_SERVICE)
            as ConnectivityManager
        mConnectivityManager.registerNetworkCallback(vpnNetworkRequest, networkCallbackHandler)
        mActive = true
        mTaskTimer.start()
        Log.e(TAG, "Started ConnectionHealth")
    }
    fun stop() {
        mActive = false
        mTaskTimer.cancel()
        val mConnectivityManager = mService.getSystemService(Context.CONNECTIVITY_SERVICE)
            as ConnectivityManager
        mConnectivityManager.unregisterNetworkCallback(networkCallbackHandler)
    }

    private fun taskDone() {
        if (!mActive) {
            return
        }
        mTaskTimer.start()
    }

    private val connectionHealthTimerMSec: Long = 30000
    /**
     * mTaskTimer
     * Queues TaskCheckConnection on the WorkerThread every 30s
     */
    private val mTaskTimer = object : CountDownTimer(
        connectionHealthTimerMSec,
        connectionHealthTimerMSec / 4
    ) {
        override fun onTick(millisUntilFinished: Long) {}
        override fun onFinish() {
            if (!mActive) {
                return
            }
            mWorker.submit(TaskCheckConnection)
        }
    }

    /**
     * networkCallbackHandler
     * This will update mVPNNetwork to match the current vpn, or set it to null on disconnect.
     */
    private val networkCallbackHandler = object : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            mVPNNetwork = network
        }
        override fun onLost(network: Network) {
            mVPNNetwork = null
        }
        override fun onUnavailable() {
            mVPNNetwork = null
        }
    }

    /**
     * vpnNetworkRequest
     * This will build a NetworkRequest that only matches
     * VPN networks on any transport.
     * Can be used for ConnectivityManager and ConnectivityDiagonsticsManager
     */
    private val vpnNetworkRequest: NetworkRequest by lazy {
        // Network requests are & - conditions, so let's only request for
        // networks that use a VPN transport type.
        NetworkRequest.Builder().apply {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                clearCapabilities()
            }
            // There might be default's set, docs are not clear
            // So let's remove all that we don't need
            removeTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
            removeTransportType(NetworkCapabilities.TRANSPORT_BLUETOOTH)
            removeTransportType(NetworkCapabilities.TRANSPORT_WIFI)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
                removeTransportType(NetworkCapabilities.TRANSPORT_LOWPAN)
            }
            removeTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                removeTransportType(NetworkCapabilities.TRANSPORT_WIFI_AWARE)
            }
            addTransportType(NetworkCapabilities.TRANSPORT_VPN)
        }.build()
    }

    /**
     * TaskCheck Connection.
     * This task will check if the Server&DNS are still reachable.
     * In case of a problem it will attempt once to reconnect to the server
     * and Ask the VPNService to switch to a fallback server, in case a reconnect did not resolve
     * the connection issue.
     */
    private val TaskCheckConnection = Runnable {
        kotlin.run {
            // capture stuff, as this the members will be set by another thread
            val gateway = mGateway; // This lives inside the tunnel
            val endpoint = mEndPoint; // This is the wg-endpoint (not in the tunnel)
            val dns = mDNS
            val fallbackEndpoint = mAltEndpoint

            // Step 1: Make sure the current active network really is the VPN
            val mConnectivityManager = mService.getSystemService(Context.CONNECTIVITY_SERVICE)
                as ConnectivityManager
            val vpnNetwork = mVPNNetwork ?: return@Runnable
            val networkCaps = mConnectivityManager.getNetworkCapabilities(vpnNetwork)
            if (networkCaps?.hasTransport(NetworkCapabilities.TRANSPORT_VPN) != true) {
                // In case it's not anymore, just end here.
                taskDone()
                return@Runnable
            }

            // Step 2: Are both the VPN-Gateway and the DNS reachable? (i.e is internet working?)
            val canReachGateway = vpnNetwork.getByName(gateway).isReachable(PING_TIMEOUT)
            val canReachDNS = vpnNetwork.getByName(dns).isReachable(PING_TIMEOUT)
            if (canReachGateway && canReachDNS) {
                // Internet should be fine :)
                mResetUsed = false
                taskDone()
                return@Runnable
            }

            if (!canReachGateway) {
                Log.e(TAG, "Failed to Reach VPN-Gateway")
            }
            if (!canReachDNS) {
                Log.e(TAG, "Failed to reach DNS")
            }

            // Step 3: We have found a connection Issue, check if we can ping the wireguard
            // endpoint on any Network, maybe the handoff from WIFI-> GSM did not work correctly
            val anyNetworkCanConnect = mConnectivityManager.allNetworks.firstOrNull() {
                it.getByName(endpoint).isReachable(PING_TIMEOUT)
            } != null
            if (anyNetworkCanConnect && canReachDNS && !mResetUsed) {
                // The server seems to be online but the connection broke,
                // Let's just try to force a reconnect ... but only once.
                mService.mainLooper.run {
                    // Silent server switch to the same server
                    mService.reconnect()
                }
                mResetUsed = true
                taskDone()
                return@Runnable
            }
            // Step 4: Either the server is unreachable or the "reconnect" did not
            // fix the issue. Try to switch to the fallback server.
            val fallbackServerIsReachable = mConnectivityManager.allNetworks.firstOrNull() {
                it.getByName(fallbackEndpoint).isReachable(PING_TIMEOUT)
            } != null
            if (fallbackServerIsReachable) {
                Log.i(TAG, "Switch to fallback VPN server")
                // We the server is online but the connection broke up, let's rest it
                mService.mainLooper.run {
                    // Silent server switch to the same server
                    mService.reconnect(true)
                    NotificationUtil.instance?.show(mService)
                }
                mResetUsed = true
                taskDone()
                return@Runnable
            }
            // If we land here: The server and the fallback are unreachable
            // Nothing we can do here to help.
            Log.e(TAG, "Both Server / Serverfallback seem to be unreachable.")
            taskDone()
        }
    }
}
