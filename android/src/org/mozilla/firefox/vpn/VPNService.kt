/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn

import android.content.Context
import android.content.Intent
import android.os.IBinder
import com.wireguard.android.backend.*
import com.wireguard.android.backend.GoBackend
import com.wireguard.config.Config
import java.lang.Exception

class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    private var mBinder: VPNServiceBinder = VPNServiceBinder(this)
    private val mBackend = GoBackend(this)
    private val mTunnel = VPNTunnel("mvpn1", mBinder)
    private var mConfig: Config? = null

    override fun onUnbind(intent: Intent?): Boolean {
        if (state == Tunnel.State.DOWN) {
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
        return mBinder
    }

    /**
     * Might be the entryPoint if the Service gets Started via an
     * Service Intent: Might be from Always-On-Vpn from Settings
     * or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.init(this)
        this.startService(Intent(this, GoBackend.VpnService::class.java))
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
                Log.e(tag, "VPN service was triggered without defining a Server or having a tunnel")
                return super.onStartCommand(intent, flags, startId)
            }
            this.mConfig = mBinder.buildConfigFromJSON(lastConfString)
        }
        turnOn(this.mConfig)
        return super.onStartCommand(intent, flags, startId)
    }

    // Invoked when the application is revoked.
    // At this moment, the VPN interface is already deactivated by the system.
    override fun onRevoke() {
        this.turnOff()
        super.onRevoke()
    }

    var statistic: Statistics? = null
        get() {
            return mBackend.getStatistics(this.mTunnel)
        }
    var state: Tunnel.State? = null
        get() {
            return mBackend.getState(this.mTunnel)
        }
    var connectionTime: Long = 0
        get() {
            return mTunnel.mConnectionTime
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
        if (!checkPermissions()) {
            Log.e(tag, "turn on was called without no permissions present!")
            mTunnel.abort()
            return
        }
        NotificationUtil.show(this) // Go foreground
        this.startService(Intent(this, GoBackend.VpnService::class.java))
        if (newConf == null && mConfig == null) {
            Log.e(tag, "Tried to start VPN with null config - abort")
        }
        if (newConf != null) {
            mConfig = newConf
        }

        // wgBackend will "DOWN" the tunnel before connecting,
        // we don't need the onchange event to be passed to the controller.
        // so set the current expected state to be down.
        mTunnel.mState = Tunnel.State.DOWN
        try {
            mBackend.setState(mTunnel, Tunnel.State.UP, mConfig)
        } catch (e: Exception) {
            mTunnel.abort()
        }
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        mBackend.setState(mTunnel, Tunnel.State.DOWN, null)
        stopForeground(false)
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
    }
}
