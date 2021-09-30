package com.adjust.sdk;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import static com.adjust.sdk.Constants.EXTRA_SYSTEM_INSTALLER_REFERRER;

public class AdjustPreinstallReferrerReceiver extends BroadcastReceiver {
  @Override
  public void onReceive(Context context, Intent intent) {
    if (intent == null) {
      return;
    }

    String referrer = intent.getStringExtra(EXTRA_SYSTEM_INSTALLER_REFERRER);
    if (referrer == null) {
      return;
    }

    Adjust.getDefaultInstance().sendPreinstallReferrer(referrer, context);
  }
}
