/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;
import android.util.Log;

import com.adjust.sdk.Adjust;
import com.adjust.sdk.AdjustConfig;
import com.adjust.sdk.LogLevel;
import com.adjust.sdk.AdjustEvent;

import org.mozilla.firefox.vpn.BuildConfig;

public class VPNApplication extends org.qtproject.qt5.android.bindings.QtApplication {

  private static VPNApplication instance;

  @Override
  public void onCreate() {
      super.onCreate();
      VPNApplication.instance = this;
  }

  public static void onVpnInit(boolean inProduction) {
      if(BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
        Log.v("Adjust", "Adjust Token: " + BuildConfig.ADJUST_SDK_TOKEN);
        String appToken = BuildConfig.ADJUST_SDK_TOKEN;
        String environment = inProduction ? AdjustConfig.ENVIRONMENT_PRODUCTION : AdjustConfig.ENVIRONMENT_SANDBOX;
        AdjustConfig config = new AdjustConfig(VPNApplication.instance, appToken, environment);
        config.setLogLevel(LogLevel.DEBUG);
        Adjust.onCreate(config);
      }
  }

  public static void trackEvent(String event) {
    if (BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
      AdjustEvent adjustEvent = new AdjustEvent(event);
      Adjust.trackEvent(adjustEvent);
    }
  }
}
