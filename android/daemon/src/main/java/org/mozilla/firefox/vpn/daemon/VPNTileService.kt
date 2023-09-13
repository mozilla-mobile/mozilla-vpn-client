/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.daemon

import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.Build
import android.os.DeadObjectException
import android.os.IBinder
import android.os.Parcel
import android.os.RemoteException
import android.service.quicksettings.Tile
import org.json.JSONObject
import org.mozilla.firefox.qt.common.CoreBinder

class VPNTileService : android.service.quicksettings.TileService() {
    enum class State {
        Unknown, Connected, Disconnected, Error
    }
    var mState: State = State.Unknown
    var mCity = ""
    private val mServiceBinder = object : CoreBinder() {
        override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
            when (code) {
                EVENTS.connected -> {
                    mState = State.Connected
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    val config = JSONObject(json)
                    mCity = config.getString("city")
                }
                EVENTS.disconnected -> mState = State.Disconnected
                EVENTS.init -> {
                    val buffer = data.createByteArray()
                    val json = buffer?.let { String(it) }
                    val config = JSONObject(json)
                    mState = if (config.getBoolean("connected")) State.Connected else State.Disconnected
                    mCity = config.getString("city")
                    if (!config.getBoolean("canActivate")) {
                        mState = State.Unknown
                    }
                }
                EVENTS.activationError -> {
                    mState = State.Error
                }
            }
            updateTile()
            return true
        }
    }
    var mService: IBinder? = null
    private val mServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(p0: ComponentName?, vpnService: IBinder?) {
            val out = Parcel.obtain()
            out.writeStrongBinder(mServiceBinder)
            try {
                // Register our IBinder Listener
                vpnService?.transact(CoreBinder.ACTIONS.registerEventListener, out, Parcel.obtain(), 0)
                // Ask the VPN Service for the current status
                vpnService?.transact(CoreBinder.ACTIONS.getStatus, out, Parcel.obtain(), 0)
                mService = vpnService
            } catch (e: DeadObjectException) {
                mService = null
            } catch (e: RemoteException) {
                e.printStackTrace()
            }
        }
        override fun onServiceDisconnected(p0: ComponentName?) {
            mState = State.Error
            updateTile()
        }
    }

    override fun onStartListening() {
        super.onStartListening()
        bindService(
            Intent(this, VPNService::class.java),
            mServiceConnection,
            BIND_AUTO_CREATE,
        )
    }

    override fun onStopListening() {
        super.onStopListening()
        unbindService(mServiceConnection)
    }

    override fun onClick() {
        when (mState) {
            State.Error -> return
            State.Unknown -> return
            State.Connected -> {
                if (isLocked) {
                    // We Should not allow the vpn to be disabled
                    // while the screen is locked
                    return
                }
                mService?.transact(CoreBinder.ACTIONS.deactivate, Parcel.obtain(), Parcel.obtain(), 0)
                mState = State.Disconnected
            }
            State.Disconnected -> {
                // No Need to check for secure :)
                mService?.transact(CoreBinder.ACTIONS.reactivate, Parcel.obtain(), Parcel.obtain(), 0)
                mState = State.Connected
            }
        }
        updateTile()
        super.onClick()
    }

    private fun updateTile() {
        var tile_state = Tile.STATE_UNAVAILABLE
        var tile_subtitle = ""
        var tile_state_description = ""

        when (mState) {
            State.Error -> {
                tile_state = Tile.STATE_UNAVAILABLE
                tile_state_description = "Error communicating with service"
            }
            State.Unknown -> {
                tile_state = Tile.STATE_UNAVAILABLE
                tile_state_description = "Not Available"
                tile_subtitle = "Disconnected"
            }
            State.Connected -> {
                tile_state = Tile.STATE_ACTIVE
                tile_state_description = "Connected with VPN"
                tile_subtitle = mCity
            }
            State.Disconnected -> {
                tile_state = Tile.STATE_INACTIVE
            }
        }

        qsTile.apply {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                subtitle = tile_subtitle
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                stateDescription = tile_state_description
            }
            state = tile_state
        }
        qsTile.updateTile()
    }
}
