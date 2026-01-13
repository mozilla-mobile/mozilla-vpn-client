/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.graphics.Insets;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.DeadObjectException;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowMetrics;
import android.view.WindowInsets;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import org.mozilla.firefox.vpn.VPNClientBinder;
import org.mozilla.firefox.vpn.daemon.VPNService;
import org.qtproject.qt.android.QtActivityBase;



public class VPNActivity extends org.qtproject.qt.android.QtActivityBase {
  private static boolean splashScreenReady = false;
  private static ViewTreeObserver.OnPreDrawListener preDrawListener = null;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    Log.d("VPNActivity", "onCreate");
          Class<?> qtNative = null;
      try {
          qtNative = Class.forName("org.qtproject.qt.android.QtNative");
          Method setActivity = qtNative.getDeclaredMethod("setActivity", Activity.class);
          setActivity.setAccessible(true);
          setActivity.invoke(null, this);  // static method → target = n
      } catch (ClassNotFoundException | IllegalAccessException | InvocationTargetException |
               NoSuchMethodException e) {
          throw new RuntimeException(e);
      }

    super.onCreate(savedInstanceState);
    instance = this;

    setupSplashScreenDelay();
    
    if (needsOrientationLock()) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    } else {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
    }
    ready = true;
  }
  private static VPNActivity instance;

 public VPNActivity() {
    super();
    Log.d("VPNActivity", "Constructor");
  }
  
  private static boolean ready = false;
  public static boolean isReady(){
    return ready;
  }

  // Set up splash screen delay using OnPreDrawListener
  private void setupSplashScreenDelay() {
    final View content = findViewById(android.R.id.content);
    preDrawListener = new ViewTreeObserver.OnPreDrawListener() {
      @Override
      public boolean onPreDraw() {
        if (splashScreenReady) {
          // Content is ready, remove listener and start drawing
          content.getViewTreeObserver().removeOnPreDrawListener(this);
          Log.d("VPNActivity", "Splash screen dismissed - app ready");
          return true;
        } else {
          // Keep splash screen visible
          Log.d("VPNActivity", "Splash screen waiting for ready signal");
          return false;
        }
      }
    };
    content.getViewTreeObserver().addOnPreDrawListener(preDrawListener);
  }

  // Call this from JNI to dismiss the splash screen and start rendering the app
  public static void dismissSplashScreen() {
    Log.d("VPNActivity", "dismissSplashScreen called from native code");
    splashScreenReady = true;
    // Trigger a redraw to invoke the OnPreDrawListener
    if (instance != null) {
      instance.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          View content = instance.findViewById(android.R.id.content);
          if (content != null) {
            content.invalidate();
          }
        }
      });
    }
  }

  public static VPNActivity getInstance() {
    Log.d("VPNActivity", "getInstance");
    if(instance != null){
      Log.d("VPNActivity", "instance exists");
    } else {
      Log.d("VPNActivity", "instance is null");
    }
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
    Log.d("VPNActivity", "getSystemService: " + name);
     try {
        Object svc = getApplicationContext().getSystemService(name);
        if (svc != null) {
            return svc;
        }
    } catch (Throwable t) {
        // swallow; we'll fall back to super below
        android.util.Log.w("VPNActivity", "getSystemService(" + name + ") via appCtx failed", t);
    }
    // For services that *require* an Activity (e.g. layout inflater tied to this theme),
    // use the normal path – but this should only happen after super.onCreate has returned.
    return super.getSystemService(name);
  }

  // Returns true if MVPN has handled the back button
  native boolean handleBackButton();

  public native void onServiceMessage(int actionCode, String body);
  public native void qtOnServiceConnected();
  public native void qtOnServiceDisconnected();
  public native void onIntentInternal();

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


  private boolean needsOrientationLock(){      
    int windowWidth;
    int windowHeight;
    
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
        // For Android 11+ use getCurrentWindowMetrics
        WindowMetrics windowMetrics = getWindowManager().getCurrentWindowMetrics();
        
        // This includes insets like status bar, nav bar, etc.
        Insets insets = windowMetrics.getWindowInsets()
                .getInsetsIgnoringVisibility(WindowInsets.Type.systemBars());
                
        // Get actual usable dimensions
        windowWidth = windowMetrics.getBounds().width() - insets.left - insets.right;
        windowHeight = windowMetrics.getBounds().height() - insets.top - insets.bottom;
    } else {
        // Fallback for older Android versions
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        windowWidth = displayMetrics.widthPixels;
        windowHeight = displayMetrics.heightPixels;
    }
    
    // The minimum width needed to display content properly (match QML value)
    // This should correspond to MZTheme.theme.desktopAppWidth in main.qml
    float density = getResources().getDisplayMetrics().density;
    int minRequiredWidth = (int)(360 * density);
    int minRequiredHeight = (int)(640 * density);
    // Check if in landscape it would fit
    return (windowHeight < minRequiredWidth || windowWidth < minRequiredHeight);
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
  private final int EVENT_VPN_CONFIG_PERMISSION_RESPONSE = 10;

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

      // At this point, the user has made a selection on the system config permission modal to either allow or not allow
      // the vpn configuration to be created, so it is safe to run activation retries via ConnectionManager::startHandshakeTimer()
      // without the possibility or re-prompting (flickering) the modal while it is currently being displayed  
      String result;
      if(resultCode == RESULT_OK) {
        result = "granted";
      }
      else {
        result = "denied";
      }
      onServiceMessage(EVENT_VPN_CONFIG_PERMISSION_RESPONSE,result);
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

  @Override
  protected void onDestroy() {
    if(bound){
      unbindService(mConnection);
    }
    ready = false;
    super.onDestroy();
  }

  /**
   * Checks the intent that opened the activtiy
   * @return a mozilla-vpn://<screenID> url
   */
  public static String getOpenerURL() {
    if (instance == null) {
      return "";
    }
    Uri maybeURI = instance.getIntent().getData();
    if (maybeURI == null) {
      // Just a normal open
      return "";
    }
    return maybeURI.toString();
  }
  @Override
  protected void onNewIntent(Intent intent) {
    // getIntent() always returns the
    // original intent the app was opened with
    // we however have no use for that
    // so let's always keep the newest one
    // and notify the Client of that Change
    setIntent(intent);
    if (nativeMethodsAvailable) {
      onIntentInternal();
    }
  }

  // Make sure we do Not Call Native Functions
  // Until the Client has told us the
  // registration of them is complete
  private static boolean nativeMethodsAvailable = false;
  private static void nativeMethodsRegistered() {
    nativeMethodsAvailable = true;
  }
}
