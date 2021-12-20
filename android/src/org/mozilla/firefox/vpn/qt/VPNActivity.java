/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;

public class VPNActivity extends org.mozilla.firefox.vpn.compat.CompatVPNActivity {
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
  public Object getSystemService(String name) {
    if (Build.VERSION.SDK_INT >= 29 && name.equals("clipboard")) {
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
}
