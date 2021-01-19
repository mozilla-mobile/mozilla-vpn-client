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
import com.mozilla.vpn.VPNTunnel
import com.wireguard.android.backend.*
import com.wireguard.config.Config
import com.wireguard.crypto.Key
import com.wireguard.crypto.KeyFormatException
import com.wireguard.android.backend.GoBackend


class VPNService : android.net.VpnService() {
    private val tag = "VPNService"
    private var mBinder: VPNServiceBinder? = null
    private val mBackend = GoBackend(this);
    private val mTunnel = VPNTunnel("mvpn1", this);
    private var mConfig:Config? = null;

    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp
     * calles bindService. Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        if (mBinder == null) {
            mBinder = VPNServiceBinder(this)
            this.startService(Intent(this, GoBackend.VpnService::class.java))
        }
        Log.v(tag, "Got Bind request")
        return mBinder
    }


    /**
     * Might be the entryPoint if the Service gets Started via an
     * Service Intent: Might be from Always-On-Vpn from Settings
     * or from Booting the device and having "connect on boot" enabled.
     */
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (mBinder == null) {
            mBinder = VPNServiceBinder(this)
            this.startService(Intent(this, GoBackend.VpnService::class.java))
        }

        intent?.let {
            if (intent.getBooleanExtra("startOnBoot", false)) {
                Log.v(tag, "Starting VPN because 'start on boot is enabled'")
            } else {
                Log.v(tag, "Starting VPN because 'always on vpn' is enabled")
            }
        }

        if (this.mConfig == null) {
            // We don't have tunnel to turn on - Try to create one with last config the service got
            val prefs = getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE);
            val lastConfString = prefs.getString("lastConf", "")
            if (lastConfString.isNullOrEmpty()) {
                // We have nothing to connect to -> Exit
                Log.e(tag, "VPN service was triggered without defining a Server or having a tunnel")
                return Service.START_NOT_STICKY
            }
            this.mConfig = mBinder?.buildConfigFromJSON(lastConfString)
        }
        val connected = turnOn(this.mConfig)
        // Try to send a status update to QT
        mBinder?.let { 
            if(connected){
                it.dispatchEvent(VPNServiceBinder.EVENTS.connected, "")
            }else{
                it.dispatchEvent(VPNServiceBinder.EVENTS.disconnected, "")
            }
        }
        return Service.START_NOT_STICKY
    }

    // Invoked when the application is revoked.
    // At this moment, the VPN interface is already deactivated by the system.
    override fun onRevoke() {
        this.turnOff();
        super.onRevoke()
    }

    var statistic: Statistics? = null
        get(){
        return mBackend.getStatistics(this.mTunnel);
    }
    var state: Tunnel.State? = null
        get(){
            return mBackend.getState(this.mTunnel);
        }

    fun onTunnelStateChange(s : Tunnel.State){
        mBinder?.let { 
            if(s == Tunnel.State.UP){
                it.dispatchEvent(VPNServiceBinder.EVENTS.connected, "")
            }else{
                it.dispatchEvent(VPNServiceBinder.EVENTS.disconnected, "")
            }
        }
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

    fun turnOn(newConf:Config?): Boolean {
        if(newConf == null && mConfig == null){
            Log.e(tag, "Tried to start VPN with null config - abort");
            return false;
        }
        if(newConf != null){
            mConfig = newConf;
        }

        // Upgrade us into a Foreground Service, by showing a Notification
        NotificationUtil.show(this);
        var tunnelState = Tunnel.State.DOWN
        try {
            tunnelState = mBackend.setState(mTunnel, Tunnel.State.UP, mConfig);
        } catch (e: Exception){
            e.message?.let { Log.e(tag, it) }
        }
        return tunnelState == Tunnel.State.UP;
    }

    fun turnOff() {
        Log.v(tag, "Try to disable tunnel")
        mBackend.setState(mTunnel,Tunnel.State.DOWN, null)
        stopForeground(false)
    }

    /**
     * Fetches the Global QTAndroidActivity and calls startActivityForResult with the given intent
     * Is used to request the VPN-Permission, if not given.
     * Actually Implemented in src/platforms/android/AndroidJNIUtils.cpp
     */
    external fun startActivityForResult(i: Intent)

}
