/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;


import android.app.Activity;
import android.os.Bundle;

import org.mozilla.firefox.vpn.GleanMetrics.GleanBuildInfo;
import org.mozilla.firefox.vpn.GleanMetrics.Pings;
import mozilla.telemetry.glean.Glean;



public class VPNApplication extends org.qtproject.qt.android.bindings.QtApplication {

  private static VPNApplication instance;

  @Override
  public void onCreate() {
      super.onCreate();
      VPNApplication.instance = this;
  }

  public static void onVpnInit(boolean inProduction, int proxyPort) {

  }

  public static void trackEvent(String event) {

  }

  public static void forget(Activity activity) {

  }

}
