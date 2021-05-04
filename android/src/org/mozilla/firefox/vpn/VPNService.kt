/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn

import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.system.OsConstants
import com.wireguard.android.util.SharedLibraryLoader
import com.wireguard.config.Config

class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    private var mBinder: VPNServiceBinder = VPNServiceBinder(this)
    private var mConfig: Config? = null
    private var mConnectionTime: Long = 0

    private var currentTunnelHandle = -1

    override fun onUnbind(intent: Intent?): Boolean {
        if (!isUp) {
            // If the Qt Client got closed while we were not connected
            // we do not need to stay as a foreground service.
            stopForeground(true)
        }
        return super.onUnbind(intent)
    }

    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp
     * calles bindService. Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        Log.init(this)
        NotificationUtil.show(this)
        Log.v(tag, "Got Bind request")

        SharedLibraryLoader.loadSharedLibrary(this, "wg-go")
        Log.i(tag, "loaded lib")
        Log.e(tag, "Wireguard Version ${wgVersion()}")

        return mBinder
    }

    /**
     * Might be the entryPoint if the Service gets Started via an
     * Service Intent: Might be from Always-On-Vpn from Settings
     * or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.init(this)
        intent?.let {
            if (intent.getBooleanExtra("startOnly", false)) {
                Log.i(tag, "Start only!")
                return super.onStartCommand(intent, flags, startId)
            }
            if (intent.getBooleanExtra("startOnBoot", false)) {
                Log.v(tag, "Starting VPN because 'start on boot is enabled'")
            } else {
                Log.v(tag, "Starting VPN because 'always on vpn' is enabled")
            }
        }
        // Go foreground if we need to connect
        NotificationUtil.show(this)

        if (this.mConfig == null) {
            // We don't have tunnel to turn on - Try to create one with last config the service got
            val prefs = Prefs.get(this)
            val lastConfString = prefs.getString("lastConf", "")
            if (lastConfString.isNullOrEmpty()) {
                // We have nothing to connect to -> Exit
                Log.e(
                    tag,
                    "VPN service was triggered without defining a Server or having a tunnel"
                )
                return super.onStartCommand(intent, flags, startId)
            }
            this.mConfig = mBinder.buildConfigFromJSON(lastConfString)
        }
        turnOn(this.mConfig!!)
        return super.onStartCommand(intent, flags, startId)
    }

    // Invoked when the application is revoked.
    // At this moment, the VPN interface is already deactivated by the system.
    override fun onRevoke() {
        this.turnOff()
        super.onRevoke()
    }

    var connectionTime: Long = 0
        get() {
            return mConnectionTime
        }

    var isUp: Boolean
        get() {
            return currentTunnelHandle >= 0
        }
        private set(value) {
            if (value) {
                mBinder.dispatchEvent(VPNServiceBinder.EVENTS.connected, "")
                mConnectionTime = System.currentTimeMillis()
                return
            }
            mBinder.dispatchEvent(VPNServiceBinder.EVENTS.disconnected, "")
            mConnectionTime = 0
        }

     /*
     * Checks if the VPN Permission is given. 
     * If the permission is given, returns true
     * Requests permission and returns false if not.
     */
    fun checkPermissions(): Boolean {
        // See https://developer.android.com/guide/topics/connectivity/vpn#connect_a_service
        // Call Prepare, if we get an Intent back, we dont have the VPN Permission
        // from the user. So we need to pass this to our main Activity and exit here.
        val intent = prepare(this)
        if (intent == null) {
            Log.e(tag, "VPN Permission Already Present")
            return true
        }
        Log.e(tag, "Requesting VPN Permission")
        return false
    }

    fun turnOn(newConf: Config?) {
        if (newConf == null) {
            return
        }
        if (!checkPermissions()) {
            Log.e(tag, "turn on was called without no permissions present!")
            isUp = false
            return
        }
        Log.i(tag, "Permission okay")
        mConfig = newConf
        if (currentTunnelHandle != -1) {
            Log.e(tag, "Tunnel already up")
            isUp = true
            return
        }

        val wgConfig: String = newConf!!.toWgUserspaceString()

        val builder = Builder()
        setupBuilder(newConf, builder)
        builder.setSession("mvpn0")
        builder.establish().use { tun ->
            if (tun == null)return
            Log.i(tag, "Go backend " + wgVersion())
            currentTunnelHandle = wgTurnOn("mvpn0", tun.detachFd(), wgConfig)
        }
        if (currentTunnelHandle < 0) {
            Log.e(tag, "Activation Error Code -> $currentTunnelHandle")
            isUp = false
            return
        }
        protect(wgGetSocketV4(currentTunnelHandle))
        protect(wgGetSocketV6(currentTunnelHandle))
        isUp = true
        NotificationUtil.show(this) // Go foreground
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        wgTurnOff(currentTunnelHandle)
        currentTunnelHandle = -1
        stopForeground(false)
        isUp = false
    }

    private fun setupBuilder(config: Config, builder: Builder) {
        // Setup Split tunnel
        for (excludedApplication in newConf.`interface`.excludedApplications)
            builder.addDisallowedApplication(excludedApplication)

        // Device IP
        for (addr in newConf.`interface`.addresses) builder.addAddress(addr.address, addr.mask)
        // DNS
        for (addr in newConf.`interface`.dnsServers) builder.addDnsServer(addr.hostAddress)
        // Add All routes the VPN may route tos
        for (peer in newConf.peers) {
            for (addr in peer.allowedIps) {
                builder.addRoute(addr.address, addr.mask)
            }
        }
        builder.allowFamily(OsConstants.AF_INET)
        builder.allowFamily(OsConstants.AF_INET6)
        builder.setMtu(newConf.`interface`.mtu.orElse(1280))

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) builder.setMetered(false)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) setUnderlyingNetworks(null)

        builder.setBlocking(true)
    }

    companion object {
        @JvmStatic
        fun startService(c: Context) {
            c.applicationContext.startService(
                Intent(c.applicationContext, VPNService::class.java).apply {
                    putExtra("startOnly", true)
                }
            )
        }

        @JvmStatic
        private external fun wgGetConfig(handle: Int): String?
        @JvmStatic
        private external fun wgGetSocketV4(handle: Int): Int
        @JvmStatic
        private external fun wgGetSocketV6(handle: Int): Int
        @JvmStatic
        private external fun wgTurnOff(handle: Int)
        @JvmStatic
        private external fun wgTurnOn(ifName: String, tunFd: Int, settings: String): Int
        @JvmStatic
        private external fun wgVersion(): String?
    }
}
