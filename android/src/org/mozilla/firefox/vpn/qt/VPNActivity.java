/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Bundle;
import android.os.DeadObjectException;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.view.KeyEvent;

import org.mozilla.firefox.vpn.VPNClientBinder;
import org.mozilla.firefox.vpn.VPNService;

import java.nio.charset.StandardCharsets;

public class VPNActivity extends org.qtproject.qt.android.bindings.QtActivity {
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    instance = this;
  }
  private static VPNActivity instance;

  public static VPNActivity getInstance() {
    return instance;
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
      onBackPressed();
      return true;
    }
    return super.onKeyDown(keyCode, event);
  }

  @Override
  public void onBackPressed() {
    try {
      if (!handleBackButton()) {
        // Move the activity into paused state if back button was pressed
        moveTaskToBack(true);
      }
    } catch (Exception e) {
    }
  }

  @Override
  public Object getSystemService (String name){

    if(Build.VERSION.SDK_INT >= 29 && name.equals("clipboard")){
      // QT will always attempt to read the clipboard if content is there.
      // since we have no use of the clipboard in android 10+
      // we _can_  return null 
      // And we defnitly should since android 12 displays clipboard access.
      return null;
    }
    return super.getSystemService(name);
  }

  // Returns true if MVPN has handled the back button
  native boolean handleBackButton();

  public native void onServiceMessage(int actionCode, String body);
  public native void qtOnServiceConnected();
  public native void qtOnServiceDisconnected();
  
  public static void connectService(){
    VPNActivity.getInstance().initServiceConnection();
  }

  public static void sendToService(int actionCode, String body) {
    VPNActivity.getInstance().dispatchParcel(actionCode, body);
  }

  private void dispatchParcel(int actionCode, String body) {
    if(!bound){
      return;
    }
    Parcel out = Parcel.obtain();
    out.writeByteArray(body.getBytes(StandardCharsets.UTF_8));
    try {
      vpnService.transact(actionCode,out,Parcel.obtain(),0);
    } catch (DeadObjectException e) {
      bound = false;
      vpnService= null;
      qtOnServiceDisconnected();
    } catch (RemoteException e) {
      e.printStackTrace();
    }
  }

  IBinder vpnService;
  boolean bound=false;
  private ServiceConnection mConnection = new ServiceConnection() {
      public void onServiceConnected(ComponentName className, IBinder service) {
          // This is called when the connection with the service has been
          // established, giving us the object we can use to
          // interact with the service.  We are communicating with the
          // service using a Messenger, so here we get a client-side
          // representation of that from the raw IBinder object.
          vpnService = service;
          bound = true;

          if(registerBinder()){
            qtOnServiceConnected();
            return;
          }
          qtOnServiceDisconnected();
      }

      public void onServiceDisconnected(ComponentName className) {
        // This is called when the connection with the service has been
        // unexpectedly disconnected -- that is, its process crashed.
        vpnService = null;
        bound = false;
        qtOnServiceDisconnected();
    }
  };

  private final int VPN_SERVICE_REGISTERBINDER = 3;
  private boolean registerBinder(){
      VPNClientBinder binder = new VPNClientBinder();
      Parcel out = Parcel.obtain();
      out.writeStrongBinder(binder);
      try {
        // Register our IBinder Listener
        vpnService.transact(VPN_SERVICE_REGISTERBINDER,out,Parcel.obtain(),0);
        return true;
      } catch (DeadObjectException e) {
            bound = false;
            vpnService= null;
      } catch (RemoteException e) {
            e.printStackTrace();
      }
    return false;
  }

  private void initServiceConnection(){
    // We already have a connection to the service,
    // just need to re-register the binder
    if(bound && vpnService.isBinderAlive() && registerBinder()){
      qtOnServiceConnected();
      return;
    }
    bindService(new Intent(this, VPNService.class), mConnection,
            Context.BIND_AUTO_CREATE);
  }


}
