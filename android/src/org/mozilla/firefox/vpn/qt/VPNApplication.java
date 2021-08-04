/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import com.adjust.sdk.Adjust;
import com.adjust.sdk.AdjustConfig;
import com.adjust.sdk.LogLevel;

import org.mozilla.firefox.vpn.BuildConfig;

public class VPNApplication extends org.qtproject.qt5.android.bindings.QtApplication {

  @Override
  public void onCreate() {
      super.onCreate();

      if(BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
        String appToken = BuildConfig.ADJUST_SDK_TOKEN;
        String environment = BuildConfig.DEBUG ? AdjustConfig.ENVIRONMENT_SANDBOX : AdjustConfig.ENVIRONMENT_PRODUCTION;
        AdjustConfig config = new AdjustConfig(this, appToken, environment);
        config.setLogLevel(LogLevel.VERBOSE);
        Adjust.onCreate(config);
      }
  }
}
