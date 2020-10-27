package com.mozilla.vpn

import android.content.Intent
import android.os.IBinder
import android.util.Log
import com.wireguard.android.backend.*
import com.wireguard.config.Config


class VPNService :   android.net.VpnService(), ServiceProxy  {
    private val tag = "VPNService"
    private var tunnel: Tunnel? = null
    private var mBinder: VPNServiceBinder? = null

    /**
     * EntryPoint for the Service, gets Called when AndroidController.cpp
     * calles bindService. Returns the [VPNServiceBinder] so QT can send Requests to it.
     */
    override fun onBind(intent: Intent?): IBinder? {
        if(mBinder == null){
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
        if(mBinder == null){
            mBinder = VPNServiceBinder(this)
        }
        return super.onStartCommand(intent, flags, startId)
    }

    override fun getStatistic(tunnel: Tunnel): Statistics {
        TODO("Not yet implemented")
    }

    fun createTunnel(conf: Config){
        this.tunnel = Tunnel("myCoolTunnel", conf)
    }

    fun turnOn(): Boolean {
        // See https://developer.android.com/guide/topics/connectivity/vpn#connect_a_service
        // Call Prepare, if we get an Intent back, we dont have the VPN Permission
        // from the user. So we need to pass this to our main Activity and exit here.
        val intent = prepare(this)
        if(intent == null){
            Log.e(tag, "VPN Permission Already Present")
        }
        else{
            Log.e(tag, "Requesting VPN Permission")
            this.startActivityForResult(intent)
            return false
        }
        val tunnel = this.tunnel ?: return false

       tunnel.tunnelHandle?.let{
           this.protect(it)
       }
        val config = tunnel.config
        val fileDescriptor = Builder().applyConfig(config).establish()

        if(fileDescriptor != null){
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
    }

    /**
     * Fetches the Global QTAndroidActivity and calls startActivityForResult with the given intent
     * Is used to request the VPN-Permission, if not given.
     * Actually Implemented in src/platforms/android/AndroidJNIUtils.cpp
     */
    external fun startActivityForResult(i: Intent)

}
