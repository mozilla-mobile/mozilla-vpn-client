/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import com.mozilla.vpn.NotificationUtil
import com.wireguard.android.backend.Statistics
import com.wireguard.android.backend.Tunnel
import com.wireguard.android.backend.VpnServiceBackend
import com.wireguard.android.backend.applyConfig
import com.wireguard.config.Config
import com.wireguard.crypto.Key
import com.wireguard.crypto.KeyFormatException


class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    var tunnel: Tunnel? = null
    private var mBinder: VPNServiceBinder? = null
    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp
     * calles bindService. Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        if (mBinder == null) {
            mBinder = VPNServiceBinder(this)
        }
        Log.v(tag, "Got Bind request")
        return mBinder
    }

    private val backend: VpnServiceBackend = VpnServiceBackend(
        object : VpnServiceBackend.VpnServiceDelegate {
            override fun protect(socket: Int): Boolean {
                return this@VPNService.protect(socket)
            }
        }
    )

    /**
     * Might be the entryPoint if the Service gets Started via an
     * Service Intent: Might be from Always-On-Vpn from Settings
     * or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (mBinder == null) {
            mBinder = VPNServiceBinder(this)
        }

        intent?.let {
            if (intent.getBooleanExtra("startOnBoot", false)) {
                Log.v(tag, "Starting VPN because 'start on boot is enabled'")
            } else {
                Log.v(tag, "Starting VPN because 'always on vpn' is enabled")
            }
        }

        if (this.tunnel == null) {
            // We don't have tunnel to turn on - Try to create one with last config the service got
            val prefs = getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE);
            val lastConfString = prefs.getString("lastConf", "")
            if (lastConfString.isNullOrEmpty()) {
                // We have nothing to connect to -> Exit
                Log.e(tag, "VPN service was triggered without defining a Server or having a tunnel")
                return Service.START_NOT_STICKY
            }
            val tunnelConfig = mBinder?.buildConfigFromJSON(lastConfString)
            createTunnel(tunnelConfig);
        }
        this.tunnel?.let {
            // If we now managed to get a tunnel, turn on!
            turnOn()
        }
        return Service.START_NOT_STICKY
    }

    // Invoked when the application is revoked.
    // At this moment, the VPN interface is already deactivated by the system.
    override fun onRevoke() {
        this.turnOff();
        super.onRevoke()
    }

    fun getStatistic(): Statistics? {
        val stats = Statistics()
        val config = tunnel?.let { backend.getConfig(it) } ?: return null
        var key: Key? = null
        var rx: Long = 0
        var tx: Long = 0
        for (line in config.split("\\n".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()) {
            if (line.startsWith("public_key=")) {
                key?.let { stats.add(it, rx, tx) }
                rx = 0
                tx = 0
                key = try {
                    Key.fromHex(line.substring(11))
                } catch (ignored: KeyFormatException) {
                    null
                }
            } else if (line.startsWith("rx_bytes=")) {
                if (key == null)
                    continue
                rx = try {
                    java.lang.Long.parseLong(line.substring(9))
                } catch (ignored: NumberFormatException) {
                    0
                }
            } else if (line.startsWith("tx_bytes=")) {
                if (key == null)
                    continue
                tx = try {
                    java.lang.Long.parseLong(line.substring(9))
                } catch (ignored: NumberFormatException) {
                    0
                }
            }
        }
        key?.let { stats.add(it, rx, tx) }
        return stats
    }

    fun createTunnel(conf: Config?) {
        if (conf == null) {
            return;
        }
        this.tunnel = Tunnel("myCoolTunnel", conf)
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
            return true;
        }
        Log.e(tag, "Requesting VPN Permission")
        this.startActivityForResult(intent)
        return false;
    }

    fun turnOn(): Boolean {
        val tunnel = this.tunnel ?: return false
        // Upgrade us into a Foreground Service, by showing a Notification
        NotificationUtil.show(this);

        tunnel.tunnelHandle?.let {
            this.protect(it)
        }
        val config = tunnel.config
        val fileDescriptor = Builder().applyConfig(config).establish()

        if (fileDescriptor != null) {
            Log.v(tag, "Got file Descriptor for VPN - Try to up")
            backend.tunnelUp(tunnel, fileDescriptor, config.toWgUserspaceString())
            return true
        }
        Log.e(tag, "Failed to get a File Descriptor for VPN")
        return false
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        this.tunnel?.let { backend.tunnelDown(it) }
        stopForeground(false)
        this.tunnel = null;
    }



    fun isUp(): Boolean {
        return tunnel?.isUp() ?: return false
    }

    /**
     * Fetches the Global QTAndroidActivity and calls startActivityForResult with the given intent
     * Is used to request the VPN-Permission, if not given.
     * Actually Implemented in src/platforms/android/AndroidJNIUtils.cpp
     */
    external fun startActivityForResult(i: Intent)

}
fun Tunnel.isUp(): Boolean {
    return this.state == Tunnel.State.Up
}

