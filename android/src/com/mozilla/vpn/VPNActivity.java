/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn;

import android.app.Activity;
import android.content.Intent;

public class VPNActivity extends org.qtproject.qt5.android.bindings.QtActivity {
  @Override
  protected void onNewIntent(Intent intent) {
    // Since there is only one instance of the qt-activity,
    // we need to save every incoming intent, so we can process it later.
    setIntent(intent);
    super.onNewIntent(intent);
  }
}
