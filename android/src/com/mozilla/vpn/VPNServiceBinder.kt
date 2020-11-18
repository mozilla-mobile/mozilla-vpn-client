/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
package com.mozilla.vpn

import android.content.Context
import android.os.Binder
import android.os.IBinder
import android.os.Parcel
import android.util.Log
import com.wireguard.config.*
import com.wireguard.crypto.Key
import java.lang.Exception
import org.json.JSONObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.lang.reflect.Field
import java.lang.reflect.Method

class VPNServiceBinder(service: VPNService) : Binder() {

    private val mService = service
    private val tag = "VPNServiceBinder"
    private val mListeners = mutableListOf<IBinder>()

    /**
     * The codes this Binder does accept in [onTransact]
     */
    object ACTIONS {
        const val activate = 1
        const val deactivate = 2
        const val registerEventListener = 3
        const val requestStatistic = 4
        const val requestLog = 5
        const val resumeActivate = 6;
        const val enableStartOnBoot =7;
    }

    /**
     * Gets called when the VPNServiceBinder gets a request from a Client.
     * The [code] determines what action is requested. - see [ACTIONS]
     * [data] may contain a utf-8 encoded json string with optional args or is null.
     * [reply] is a pointer to a buffer in the clients memory, to reply results.
     * we use this to send result data.
     *
     * returns true if the [code] was accepted
     */
    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        Log.i(tag, "GOT TRANSACTION $code")

        when (code) {
            ACTIONS.activate -> {
                try {
                    // [data] is here a json containing the wireguard conf
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    // Store the config in case the service gets
                    // asked boot vpn from the OS
                    val prefs = mService.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE);
                    prefs.edit()
                        .putString("lastConf",json)
                        .apply()

                    Log.v(tag,"Stored new Tunnel config in Service")
                    val config = buildConfigFromJSON(json)

                    var forSwitching = false;
                    json?.let {
                        val obj = JSONObject(it)
                        forSwitching = obj.getBoolean("forSwitching")
                    }
                    if(forSwitching){
                        // In case the activation is for switching purposes
                        // We never turned the vpn off so far.
                        this.mService.turnOff();
                    }

                    this.mService.createTunnel(config)
                    if(!mService.checkPermissions()){
                        // The Permission Promt was already
                        // send, in case it's accepted we will 
                        // recive ACTIONS.resumeActivate
                        return true;
                    }
                    if (this.mService.turnOn()) {
                        dispatchEvent(EVENTS.connected, "")
                    } else {
                        dispatchEvent(EVENTS.disconnected, "")
                    }
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                    dispatchEvent(EVENTS.disconnected, "")
                }
                return true
            }
            ACTIONS.resumeActivate -> {
                // [data] is empty
                // Activate the current tunnel
                if(!mService.checkPermissions()){
                    return true;
                }
                if (this.mService.turnOn()) {
                    dispatchEvent(EVENTS.connected, "")
                } else {
                    dispatchEvent(EVENTS.disconnected, "")
                }
                return true;
            }
            ACTIONS.deactivate -> {
                // [data] here is empty
                this.mService.turnOff()
                dispatchEvent(EVENTS.disconnected, "")
                return true
            }
            ACTIONS.registerEventListener -> {
                // [data] contains the Binder that we need to dispatch the Events
                val binder = data.readStrongBinder()
                mListeners.add(binder)
                Log.d(tag, "Registered ${mListeners.size} EventListeners")
                if(mService.isUp()){
                    dispatchEvent(EVENTS.init, "connected")
                }else{
                    dispatchEvent(EVENTS.init, "disconnected")
                }
            }
            ACTIONS.requestStatistic -> {
                val statistics = this.mService.getStatistic()
                val obj = JSONObject()
                obj.put("totalRX", statistics?.totalRx())
                obj.put("totalTX", statistics?.totalTx())
                dispatchEvent(EVENTS.statisticUpdate, obj.toString())
            }
            ACTIONS.requestLog ->{
                // Grabs all the Logs and dispatch new Log Event
                val process = Runtime.getRuntime().exec("logcat -d");
                val bufferedReader = BufferedReader(
                    InputStreamReader(process.inputStream))
                val allText = bufferedReader.use(BufferedReader::readText)
                dispatchEvent(EVENTS.backendLogs, allText)

            }
            ACTIONS.enableStartOnBoot ->{
                // Sets the Start on boot pref data is here a QVariant Byte
                val startOnBootEnabled = data.readByte().equals(true) // there is no byte.toBool?
                Log.v(tag,"Set ServicePref Start on boot to -> $startOnBootEnabled")
                val prefs = mService.getSharedPreferences("com.mozilla.vpn.prefrences", Context.MODE_PRIVATE);
                prefs.edit()
                    .putBoolean("startOnBoot",startOnBootEnabled)
                    .apply()
            }

            else -> {
                Log.e(tag, "Received invalid bind request \t Code -> $code")
                // If we're hitting this there is probably something wrong in the client.
                return false
            }
        }
        return false
    }

    /**
     * Dispatches an Event to all registered Binders
     * [code] the Event that happened - see [EVENTS]
     * To register an Eventhandler use [onTransact] with
     * [ACTIONS.registerEventListener]
     */
    private fun dispatchEvent(code: Int, payload: String) {
        mListeners.forEach {
           if (it.isBinderAlive) {
               val data = Parcel.obtain()
               data.writeByteArray(payload.toByteArray(charset("UTF-8")))
               it.transact(code, data, Parcel.obtain(), 0)
           }
        }
    }

    /**
     *  The codes we Are Using in case of [dispatchEvent]
     */
    object EVENTS {
        const val init= 0
        const val connected = 1
        const val disconnected = 2
        const val statisticUpdate = 3
        const val backendLogs = 4
    }

    /**
     * Create a Wireguard [Config]  from a [json] string -
     * The [json] will be created in AndroidController.cpp
     */
    fun buildConfigFromJSON(json: String?): Config {
        val confBuilder = Config.Builder()
        if (json == null) {
            return confBuilder.build()
        }
        val obj = JSONObject(json)
        val jServer = obj.getJSONObject("server")
        val peerBuilder = Peer.Builder()
        val ep =
            InetEndpoint.parse(jServer.getString("ipv4AddrIn") + ":" + jServer.getString("port"))
        peerBuilder.setEndpoint(ep)
        peerBuilder.setPublicKey(Key.fromBase64(jServer.getString("publicKey")))

        val jAllowedIPList = obj.getJSONArray("allowedIPs")
        if (jAllowedIPList.length() == 0) {
            val internet = InetNetwork.parse("0.0.0.0/0") // aka The whole internet.
            peerBuilder.addAllowedIp(internet)
        } else {
            (0 until jAllowedIPList.length()).toList().forEach {
                val network = InetNetwork.parse(jAllowedIPList.getString(it))
                peerBuilder.addAllowedIp(network)
            }
        }

        confBuilder.addPeer(peerBuilder.build())

        val privateKey = obj.getJSONObject("keys").getString("privateKey")
        val jDevice = obj.getJSONObject("device")
        val ifaceBuilder = Interface.Builder()
        ifaceBuilder.parsePrivateKey(privateKey)
        ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv4Address")))
        ifaceBuilder.addAddress(InetNetwork.parse(jDevice.getString("ipv6Address")))
        confBuilder.setInterface(ifaceBuilder.build())
        return confBuilder.build()
    }
}
