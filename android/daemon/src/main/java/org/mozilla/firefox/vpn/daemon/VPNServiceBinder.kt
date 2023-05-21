/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.Intent
import android.os.Binder
import android.os.DeadObjectException
import android.os.IBinder
import android.os.Parcel
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import org.json.JSONObject
import kotlin.Exception

class VPNServiceBinder(service: VPNService) : Binder() {

    private val mService = service
    private val tag = "VPNServiceBinder"

    private val mListeners = ArrayList<IBinder>()
    private var mResumeConfig: JSONObject? = null

    /** The codes this Binder does accept in [onTransact] */
    object ACTIONS {
        const val activate = 1
        const val deactivate = 2
        const val registerEventListener = 3
        const val requestStatistic = 4
        const val requestCleanupLog = 6
        const val resumeActivate = 7
        const val setNotificationText = 8
        const val recordEvent = 10
        const val getStatus = 13
        const val setStartOnBoot = 15
        const val reactivate = 16
        const val clearStorage = 17
        const val setGleanUploadEnabled = 18
    }

    /**
     * Gets called when the VPNServiceBinder gets a request from a Client. The [code] determines
     * what action is requested. - see [ACTIONS] [data] may contain a utf-8 encoded json string with
     * optional args or is null. [reply] is a pointer to a buffer in the clients memory, to reply
     * results. we use this to send result data.
     *
     * returns true if the [code] was accepted
     */
    override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
        when (code) {
            ACTIONS.activate -> {
                try {
                    Log.i(tag, "Activation Requested")
                    // [data] is here a json containing the wireguard conf
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    val config = json?.let { JSONObject(it) }
                    if (config == null) {
                        Log.e(tag, "Client provided config was not parsable")
                        return true
                    }
                    val permissionIntent: Intent? = mService.checkPermissions()
                    if (permissionIntent != null) {
                        mResumeConfig = config
                        val permissionParcel = Parcel.obtain()
                        permissionIntent.writeToParcel(permissionParcel, 0)
                        dispatchEvent(EVENTS.permissionRequired, permissionParcel)
                        // The Permission prompt was already
                        // send, in case it's accepted we will
                        // receive ACTIONS.resumeActivate
                        return true
                    }
                    this.mService.turnOn(config)
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                    Log.stack(tag, e.stackTrace)
                    dispatchEvent(EVENTS.activationError, e.localizedMessage)
                }
                return true
            }
            ACTIONS.resumeActivate -> {
                // [data] is empty
                // Activate the current tunnel
                try {
                    Log.i(tag, "Resume Activation requested")
                    mResumeConfig?.let { this.mService.turnOn(it) }
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                    dispatchEvent(EVENTS.activationError, e.localizedMessage)
                }
                return true
            }
            ACTIONS.reactivate -> {
                // [data] is empty
                // Activate the tunnel with the last config
                try {
                    this.mService.reconnect()
                } catch (e: Exception) {
                    Log.e(tag, "An Error occurred while enabling the VPN: ${e.localizedMessage}")
                    dispatchEvent(EVENTS.activationError, e.localizedMessage)
                }
                return true
            }
            ACTIONS.deactivate -> {
                Log.i(tag, "Deactivation requested")
                // [data] here is empty
                this.mService.turnOff()
                return true
            }
            ACTIONS.registerEventListener -> {
                Log.i(tag, "requested to add an Event Listener")
                // [data] contains the Binder that we need to dispatch the Events
                val binder = data.readStrongBinder()
                mListeners.add(binder)
                Log.i(tag, "Registered binder now: ${mListeners.size} Binders")

                if (!Prefs.get(mService).contains("glean_enabled")) {
                    Log.i(tag, "Requesting Glean upload enabled state. No value in storage.")
                    dispatchEvent(
                        VPNServiceBinder.EVENTS.requestGleanUploadEnabledState,
                        "",
                        binder,
                    )
                }

                return true
            }
            ACTIONS.getStatus -> {
                val obj = JSONObject()
                obj.put("connected", mService.isUp)
                obj.put("time", mService.connectionTime)
                obj.put("city", mService.cityname)
                obj.put("canActivate", mService.canActivate)
                obj.put("connection-health-status", mService.mConnectionHealth.getStatusString())
                dispatchEvent(EVENTS.init, obj.toString())
                return true
            }
            ACTIONS.requestStatistic -> {
                dispatchEvent(EVENTS.statisticUpdate, mService.status.toString())
                return true
            }
            ACTIONS.requestCleanupLog -> {
                Log.clearFile()
                return true
            }
            ACTIONS.setNotificationText -> {
                val buffer = data.createByteArray()
                val json = buffer?.let { String(it) }
                if (json.isNullOrEmpty()) {
                    return false
                }
                try {
                    val message = Json.decodeFromString<ClientNotification>(json)
                    mService.mNotificationHandler.setNotificationText(message)
                } catch (e: Exception) {
                    e.message?.let { Log.e(tag, it) }
                }
                return true
            }
            ACTIONS.setStartOnBoot -> {
                val buffer = data.createByteArray()
                val json = buffer?.let { String(it) }
                val args = JSONObject(json)
                val value = args.getBoolean("startOnBoot")
                Prefs.get(mService)
                    .edit()
                    .apply() { putBoolean(BootReceiver.START_ON_BOOT, value) }
                    .apply()
            }
            ACTIONS.clearStorage -> {
                mService.clearConfig()
            }
            ACTIONS.setGleanUploadEnabled -> {
                val buffer = data.createByteArray()
                val json = buffer?.let { String(it) }
                val args = JSONObject(json)
                mService.setGleanUploadEnabled(args.getBoolean("uploadEnabled"))
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
     * Dispatches an Event to all registered Binders [code] the Event that happened - see [EVENTS]
     * To register an Eventhandler use [onTransact] with [ACTIONS.registerEventListener] When
     * [targetBinder] is Provided, it will only dispatch the event to it.
     */
    fun dispatchEvent(code: Int, payload: String = "", targetBinder: IBinder? = null) {
        val data = Parcel.obtain()
        data.writeByteArray(payload?.toByteArray(charset("UTF-8")))
        dispatchEvent(code, data, targetBinder)
    }
    fun dispatchEvent(code: Int, data: Parcel, targetBinder: IBinder? = null) {
        targetBinder?.let {
            try {
                it.transact(code, data, Parcel.obtain(), 0)
            } catch (e: DeadObjectException) {
                Log.e(tag, "Attempted to dispatch event '$code' to dead binder. Removing.")
                // The binder is not alive, so we can remove it
                // from the listeners list, if present.
                mListeners.remove(it)
            }
            return
        }
        val deadBinders = ArrayList<IBinder>()
        mListeners.forEach {
            if (it.isBinderAlive) {
                try {
                    it.transact(code, data, Parcel.obtain(), 0)
                } catch (e: DeadObjectException) {
                    // If the QT Process is killed (not just inactive)
                    // we cant access isBinderAlive, so nothing to do here.
                    deadBinders.add(it)
                }
            } else {
                deadBinders.add(it)
            }
        }
        if (deadBinders.size > 0) {
            mListeners.removeAll(deadBinders)
            Log.i(tag, "Removed ${deadBinders.size} dead Binders")
        }
    }

    val isClientAttached: Boolean
        get() {
            return try {
                mListeners.any { it.isBinderAlive }
            } catch (e: DeadObjectException) {
                false
            }
        }

    /** The codes we Are Using in case of [dispatchEvent] */
    object EVENTS {
        const val init = 0
        const val connected = 1
        const val disconnected = 2
        const val statisticUpdate = 3
        const val activationError = 5
        const val permissionRequired = 6
        const val requestGleanUploadEnabledState = 7
    }
}
