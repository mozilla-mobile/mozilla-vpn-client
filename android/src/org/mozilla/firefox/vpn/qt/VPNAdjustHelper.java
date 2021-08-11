/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import com.adjust.sdk.Adjust;
import com.adjust.sdk.AdjustEvent;

import org.mozilla.firefox.vpn.BuildConfig;

public class VPNAdjustHelper {
  
  private VPNAdjustHelper() {}

  public static void trackEvent(String event) {
    if (BuildConfig.ADJUST_SDK_TOKEN != null && !BuildConfig.ADJUST_SDK_TOKEN.isEmpty()) {
      AdjustEvent adjustEvent = new AdjustEvent(event);
      Adjust.trackEvent(adjustEvent);
    }
  }
}
