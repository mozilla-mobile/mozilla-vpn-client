/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import com.adjust.sdk.Adjust;
import com.adjust.sdk.AdjustConfig;
import com.adjust.sdk.LogLevel;
import com.adjust.sdk.AdjustEvent;
import com.adjust.sdk.AdjustFactory;
import android.app.Activity;
import android.os.Bundle;

import com.adjust.BuildConfig;

public class VPNApplication extends org.qtproject.qt.android.bindings.QtApplication {

  private static VPNApplication instance;

  @Override
  public void onCreate() {
      super.onCreate();
      VPNApplication.instance = this;
  }

  public static void onVpnInit(boolean inProduction, int proxyPort) {
    if (BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
      String appToken = BuildConfig.ADJUST_SDK_TOKEN;
      String environment =
          inProduction ? AdjustConfig.ENVIRONMENT_PRODUCTION : AdjustConfig.ENVIRONMENT_SANDBOX;
      AdjustConfig config = new AdjustConfig(VPNApplication.instance, appToken, environment);
      config.setLogLevel(LogLevel.DEBUG);
      config.setSendInBackground(true);
      AdjustFactory.setBaseUrl("http://127.0.0.1:" + proxyPort);
      AdjustFactory.setGdprUrl("http://127.0.0.1:" + proxyPort);
      Adjust.onCreate(config);

      VPNApplication.instance.registerActivityLifecycleCallbacks(new AdjustLifecycleCallbacks());
    }
  }

  public static void trackEvent(String event) {
    if (BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
      AdjustEvent adjustEvent = new AdjustEvent(event);
      Adjust.trackEvent(adjustEvent);
    }
  }

  public static void forget(Activity activity) {
    if (BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
      Adjust.gdprForgetMe(activity);
    }
  }

  private static final class AdjustLifecycleCallbacks implements ActivityLifecycleCallbacks {
      @Override
      public void onActivityResumed(Activity activity) {
          Adjust.onResume();
      }

      @Override
      public void onActivityPaused(Activity activity) {
          Adjust.onPause();
      }

      @Override
      public void onActivityStopped(Activity activity) {
      }

      @Override
      public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
      }

      @Override
      public void onActivityDestroyed(Activity activity) {
      }

      @Override
      public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
      }

      @Override
      public void onActivityStarted(Activity activity) {
      }
  }
}
