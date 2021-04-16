/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;

public class VPNActivity extends org.qtproject.qt5.android.bindings.QtActivity {
  @Override
  protected void onNewIntent(Intent intent) {
    // Since there is only one instance of the qt-activity,
    // we need to save every incoming intent, so we can process it later.
    setIntent(intent);
    super.onNewIntent(intent);
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
    Log.d("QT-Ac", "BACK BUTTON HANDELED");
    try {
      if (!handleBackButton()) {
        // Move the activity into paused state if back button was pressed
        super.onBackPressed();
      }
    } catch (Exception e) {
    }
  }

  // Returns true if MVPN has handled the back button
  native boolean handleBackButton();
}
