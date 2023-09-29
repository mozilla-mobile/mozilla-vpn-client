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
import android.view.Window;
import android.view.WindowManager;
import java.nio.charset.StandardCharsets;
import org.mozilla.firefox.vpn.VPNClientBinder;
import org.mozilla.firefox.vpn.daemon.VPNService;

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

  // Prevents Screenshots, if isSensitive is true.
  public static void setScreenSensitivity(boolean isSensitive) {
    if (isSensitive) {
      VPNActivity.getInstance().getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
    } else {
      VPNActivity.getInstance().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }
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

  private boolean registerBinder(){
      VPNClientBinder binder = new VPNClientBinder();
      Parcel out = Parcel.obtain();
      out.writeStrongBinder(binder);
      try {
        // Register our IBinder Listener
        vpnService.transact(ACTION_REGISTER_LISTENER,out,Parcel.obtain(),0);
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
  // TODO: VPN-5528 we need to port this class to kotlin
  // so we can use that Shared Code
  private final int PERMISSION_TRANSACTION = 1337;
  private final int ACTION_REGISTER_LISTENER = 3;
  private final int ACTION_RESUME_ACTIVATE = 7;
  private final int ACTION_NOTIFICATION_PROMPT_SENT = 19;
  private final int EVENT_PERMISSION_REQURED = 6;
  private final int EVENT_DISCONNECTED = 2;
  private final int EVENT_ONBOARDING_COMPLETED = 9;

  public void onPermissionRequest(int code, Parcel data) {
    if(code != EVENT_PERMISSION_REQURED){
      return;
    }
    Intent x = new Intent();
    x.readFromParcel(data);
    startActivityForResult(x,PERMISSION_TRANSACTION);
  }
  public void onNotificationPermissionRequest() {
    String permissions[] = new String[1];
    permissions[0] = "android.permission.POST_NOTIFICATIONS";
    requestPermissions(permissions, 1);
    dispatchParcel(ACTION_NOTIFICATION_PROMPT_SENT, "");
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    if(requestCode == PERMISSION_TRANSACTION){
      // THATS US!
      if( resultCode == RESULT_OK ){
        // Prompt accepted, tell service to retry.
        dispatchParcel(ACTION_RESUME_ACTIVATE,"");
      }else{
        // Tell the Client we've disconnected
        onServiceMessage(EVENT_DISCONNECTED,"");
        onServiceMessage(EVENT_ONBOARDING_COMPLETED,"");
      }
      return;
    }
    super.onActivityResult(requestCode, resultCode, data);
  }
}
