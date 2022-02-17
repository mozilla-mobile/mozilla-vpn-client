/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 package org.mozilla.firefox.vpn

 import android.content.ComponentName
 import android.content.Context
 import android.content.Intent
 import android.content.ServiceConnection
 import android.os.*
 import android.service.quicksettings.Tile
 import android.system.OsConstants
 import com.wireguard.android.util.SharedLibraryLoader
 import com.wireguard.config.*
 import com.wireguard.crypto.Key
 import org.json.JSONObject
 import android.service.quicksettings.TileService
 import androidx.annotation.RequiresApi
 import org.mozilla.firefox.vpn.qt.VPNActivity
 import java.io.FileDescriptor
 import java.io.PrintWriter

class VPNTileService : android.service.quicksettings.TileService() {
  enum class State {
   Unknown, Connected, Disconnected, Error
  }
 var mState :State = State.Unknown
 var mCriticalError = false;
 var mCity = "";
 private val mServiceBinder = object : Binder(){
   override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
    when(code){
      VPNServiceBinder.EVENTS.connected -> {
        mState = State.Connected
        val buffer = data.createByteArray()
        val json = buffer?.let { String(it) }
        val config = JSONObject(json)
        mCity = config.getString("city")
      }
      VPNServiceBinder.EVENTS.disconnected -> mState = State.Disconnected
      VPNServiceBinder.EVENTS.init -> {
       val buffer = data.createByteArray()
       val json = buffer?.let { String(it) }
       val config = JSONObject(json)
       mState = if(config.getBoolean("connected")) State.Connected else State.Disconnected;
       mCity = config.getString("city")
       
      }
    }
    updateTile();
    return true
   }
 }
 var mService : IBinder? = null;
 private val mServiceConnection = object : ServiceConnection {
  override fun onServiceConnected(p0: ComponentName?, vpnService: IBinder?) {
   val out = Parcel.obtain()
   out.writeStrongBinder(mServiceBinder)
   try {
    // Register our IBinder Listener
    vpnService?.transact(3, out, Parcel.obtain(), 0)
    mService = vpnService
   } catch (e: DeadObjectException) {
    mService = null
   } catch (e: RemoteException) {
    e.printStackTrace()
   }
  }
  override fun onServiceDisconnected(p0: ComponentName?) {
    mState=State.Error
    updateTile()
   }
 }

 override fun onStartListening() {
  super.onStartListening();
  bindService(
   Intent(this, VPNService::class.java), mServiceConnection, BIND_AUTO_CREATE
  )
 }

 override fun onStopListening() {
  super.onStopListening()
  unbindService(mServiceConnection);
 }

 override fun onClick() {
  when(mState){
   State.Error -> return;
   State.Unknown -> return;
   State.Connected ->{
     if(isLocked){
       // We Should not allow the vpn to be disabled
       // while the screen is locked
       return;
     }
     mService?.transact(VPNServiceBinder.ACTIONS.deactivate, Parcel.obtain(), Parcel.obtain(),0)
     mState=State.Disconnected
   }
   State.Disconnected ->{
    // No Need to check for secure :)
    mService?.transact(VPNServiceBinder.ACTIONS.reactivate, Parcel.obtain(), Parcel.obtain(),0)
    mState=State.Connected
   }
  }
  updateTile();
  super.onClick()
 }


 private fun updateTile(){
  qsTile.apply{
   when (mState){
    State.Error ->{
      state = Tile.STATE_UNAVAILABLE
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
       stateDescription = "Error CommuniCating with service"
      };
    }
    State.Unknown ->{
     state = Tile.STATE_UNAVAILABLE
     if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
      stateDescription = "Not Connected With Service"
     };
    }
    State.Connected ->{
     state = Tile.STATE_ACTIVE
     if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
      stateDescription = "Connected with VPN"
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
       subtitle =mCity
      }
     };
    }
    State.Disconnected ->{
     state = Tile.STATE_INACTIVE
     if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
      subtitle ="Disconnected"
     }
     if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
      stateDescription = "Not Connected with VPN"
     };
    }
   }
  }
  qsTile.updateTile()

 }
}
 