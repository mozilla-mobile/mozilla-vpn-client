/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn

import android.R
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import androidx.core.content.ContextCompat
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
     * Service Intent (Settings or vice versa)
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (mBinder == null) {
            mBinder = VPNServiceBinder(this)
        }
        return super.onStartCommand(intent, flags, startId)
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

    fun createTunnel(conf: Config) {
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

        tunnel.tunnelHandle?.let {
            this.protect(it)
        }
        val config = tunnel.config
        val fileDescriptor = Builder().applyConfig(config).establish()

        if (fileDescriptor != null) {
            Log.v(tag, "Got file Descriptor for VPN - Try to up")
            backend.tunnelUp(tunnel, fileDescriptor, config.toWgUserspaceString())
            this.startSticky()
            return true
        }
        Log.e(tag, "Failed to get a File Descriptor for VPN")
        return false
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        this.tunnel?.let { backend.tunnelDown(it) }
        stopForeground(true)
    }

    val NOTIFICATION_CHANNEL_ID = "com.mozilla.vpnNotification"
    val CONNECTED_NOTIFICATION_ID = 1337

    /*
    * Creates a Sticky Notification for this
    * Service and Calls startForeground to 
    * make sure we cant get closed as long
    * as we're connected
     */
    fun startSticky() {
        // For Android 8+ We need to Register a Notification Channel
        val notificationManager: NotificationManager =
            this.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "vpn"
            val descriptionText = "  "
            val importance = NotificationManager.IMPORTANCE_LOW
            val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            // Register the channel with the system
            notificationManager.createNotificationChannel(channel)
        }
        // Create the Intent that Should be Fired if the User Clicks the notification
        val mainActivityName = "org.qtproject.qt5.android.bindings.QtActivity";
        val activity = Class.forName(mainActivityName);
        val intent = Intent(this, activity)
        val pendingIntent = PendingIntent.getActivity(this, 0, intent, 0)

        val builder = NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID)
            .setSmallIcon(com.mozilla.vpn.R.drawable.ic_logo_on)
            .setContentTitle("Todo: Connected Title")
            .setContentText("Todo: you're connected")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(pendingIntent)
        startForeground(CONNECTED_NOTIFICATION_ID, builder.build())
    }

    /**
     * Fetches the Global QTAndroidActivity and calls startActivityForResult with the given intent
     * Is used to request the VPN-Permission, if not given.
     * Actually Implemented in src/platforms/android/AndroidJNIUtils.cpp
     */
    external fun startActivityForResult(i: Intent)
}
