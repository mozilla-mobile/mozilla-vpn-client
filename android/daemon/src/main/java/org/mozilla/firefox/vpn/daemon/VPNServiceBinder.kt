/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon
import android.os.Binder
import android.os.DeadObjectException
import android.os.IBinder
import android.os.Parcel
import org.json.JSONObject
import java.lang.Exception

class VPNServiceBinder(service: VPNService) : Binder() {

    private val mService = service
    private val tag = "VPNServiceBinder"
    private var mListener: IBinder? = null
    private var mResumeConfig: JSONObject? = null

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
        const val setStrings = 9
        const val recordEvent = 10
        const val sendGleanPings = 11
        const val gleanUploadEnabledChanged = 12
        const val controllerInit = 13
        const val gleanSetSourceTags = 14
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
        when (code) {
            ACTIONS.activate -> {
                try {
                    Log.i(tag, "Activation Requested")
                    // [data] is here a json containing the wireguard conf
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    val config = JSONObject(json)

                    if (!mService.checkPermissions()) {
                        mResumeConfig = config
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
                mListener = binder
                return true
            }
            ACTIONS.controllerInit -> {
                val obj = JSONObject()
                obj.put("connected", mService.isUp)
                obj.put("time", mService.connectionTime)
                dispatchEvent(EVENTS.init, obj.toString())
                return true
            }

            ACTIONS.requestStatistic -> {
                dispatchEvent(EVENTS.statisticUpdate, mService.status.toString())
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
                NotificationUtil.get(mService)?.update(data)
                return true
            }
            ACTIONS.setStrings -> {
                NotificationUtil.get(mService)?.updateStrings(data, mService)
                return true
            }
            ACTIONS.recordEvent -> {
                val buffer = data.createByteArray()
                val json = buffer?.let { String(it) }
                val event = JSONObject(json)
                mService.mGlean.recordEvent(event)
                return true
            }
            ACTIONS.sendGleanPings -> {
                mService.mGlean.sendGleanMainPing()
                return true
            }
            ACTIONS.gleanUploadEnabledChanged -> {
                val buffer = data.createByteArray()
                val json = buffer?.let { String(it) }
                val args = JSONObject(json)
                mService.mGlean.setGleanUploadEnabled(args.getBoolean("enabled"))
                return true
            }
            ACTIONS.gleanSetSourceTags -> {
                val buffer = data.createByteArray()
                val list = buffer?.let { String(it) }
                mService.mGlean.setGleanSourceTag(list)
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
}
