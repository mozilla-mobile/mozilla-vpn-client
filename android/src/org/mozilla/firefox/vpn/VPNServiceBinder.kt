/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn
import android.os.Binder
import android.os.DeadObjectException
import android.os.IBinder
import android.os.Parcel
import com.wireguard.config.*
import com.wireguard.crypto.Key
import org.json.JSONObject
import java.lang.Exception

class VPNServiceBinder(service: VPNService) : Binder() {

    private val mService = service
    private val tag = "VPNServiceBinder"
    private var mListener: IBinder? = null
    private var mResumeConfig: Config? = null

    /**
     * The codes this Binder does accept in [onTransact]
     */
    object ACTIONS {
        const val activate = 1
        const val deactivate = 2
        const val registerEventListener = 3
        const val requestStatistic = 4
        const val requestGetLog = 5
        const val requestCleanupLog = 6
        const val resumeActivate = 7
        const val setNotificationText = 8
        const val setFallBackNotification = 9
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
                    Log.i(tag, "Activiation Requested, parsing Config")
                    // [data] is here a json containing the wireguard conf
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    // Store the config in case the service gets
                    // asked boot vpn from the OS
                    val prefs = Prefs.get(mService)
                    prefs.edit()
                        .putString("lastConf", json)
                        .apply()

                    Log.sensitive(tag, json)

                    Log.v(tag, "Stored new Tunnel config in Service")
                    val config = buildConfigFromJSON(json)

                    if (!mService.checkPermissions()) {
                        mResumeConfig = config
                        // The Permission prompt was already
                        // send, in case it's accepted we will 
                        // recive ACTIONS.resumeActivate
                        return true
                    }
                    this.mService.turnOn(config)
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                    dispatchEvent(EVENTS.activationError, e.localizedMessage)
                }
                return true
            }

            ACTIONS.resumeActivate -> {
                // [data] is empty
                // Activate the current tunnel
                try {
                    this.mService.turnOn(mResumeConfig)
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                }
                return true
            }

            ACTIONS.deactivate -> {
                // [data] here is empty
                this.mService.turnOff()
                return true
            }

            ACTIONS.registerEventListener -> {
                // [data] contains the Binder that we need to dispatch the Events
                val binder = data.readStrongBinder()
                mListener = binder
                val obj = JSONObject()
                obj.put("connected", mService.isUp)
                obj.put("time", mService.connectionTime)
                dispatchEvent(EVENTS.init, obj.toString())
                return true
            }

            ACTIONS.requestStatistic -> {
                val obj = JSONObject()
                obj.put("totalRX", mService.totalRx)
                obj.put("totalTX", mService.totalTx)
                dispatchEvent(EVENTS.statisticUpdate, obj.toString())
                return true
            }

            ACTIONS.requestGetLog -> {
                // Grabs all the Logs and dispatch new Log Event
                dispatchEvent(EVENTS.backendLogs, Log.getContent())
                return true
            }
            ACTIONS.requestCleanupLog -> {
                Log.clearFile()
                return true
            }
            ACTIONS.setNotificationText -> {
                NotificationUtil.update(data)
                return true
            }
            ACTIONS.setFallBackNotification -> {
                NotificationUtil.saveFallBackMessage(data, mService)
                return true
            }
            IBinder.LAST_CALL_TRANSACTION -> {
                Log.e(tag, "The OS Requested to shut down the VPN")
                this.mService.turnOff()
                return true
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
    fun dispatchEvent(code: Int, payload: String?) {
        try {
            mListener?.let {
                if (it.isBinderAlive) {
                    val data = Parcel.obtain()
                    data.writeByteArray(payload?.toByteArray(charset("UTF-8")))
                    it.transact(code, data, Parcel.obtain(), 0)
                }
            }
        } catch (e: DeadObjectException) {
            // If the QT Process is killed (not just inactive)
            // we cant access isBinderAlive, so nothing to do here.
        }
    }

    /**
     *  The codes we Are Using in case of [dispatchEvent]
     */
    object EVENTS {
        const val init = 0
        const val connected = 1
        const val disconnected = 2
        const val statisticUpdate = 3
        const val backendLogs = 4
        const val activationError = 5
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
        ifaceBuilder.addDnsServer(InetNetwork.parse(obj.getString("dns")).address)
        val jExcludedApplication = obj.getJSONArray("excludedApps")
        (0 until jExcludedApplication.length()).toList().forEach {
            val appName = jExcludedApplication.get(it).toString()
            ifaceBuilder.excludeApplication(appName)
        }
        confBuilder.setInterface(ifaceBuilder.build())
        return confBuilder.build()
    }
}
