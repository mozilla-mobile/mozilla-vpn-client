package com.adjust.sdk;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import java.util.concurrent.atomic.AtomicBoolean;

public class InstallReferrerHuawei {
  /**
   * Huawei install referrer provider content authority.
   */
  private static final String REFERRER_PROVIDER_AUTHORITY = "com.huawei.appmarket.commondata";

  /**
   * Huawei install referrer provider content uri.
   */
  private static final String REFERRER_PROVIDER_URI =
      "content://" + REFERRER_PROVIDER_AUTHORITY + "/item/5";

  /**
   * Adjust logger instance.
   */
  private ILogger logger;

  /**
   * Application context.
   */
  private Context context;

  /**
   * Weak reference to ActivityHandler instance.
   */
  private final InstallReferrerReadListener referrerCallback;

  /**
   * Boolean indicating whether service should be tried to read.
   * Either because it has not yet tried,
   *  or it did and it was successful
   *  or it did, was not successful, but it should not retry
   */
  private final AtomicBoolean shouldTryToRead;

  /**
   * Default constructor.
   *
   * @param context         Application context
   * @param referrerCallback Callback for referrer information
   */
  public InstallReferrerHuawei(
      final Context context, final InstallReferrerReadListener referrerCallback) {
    this.logger = AdjustFactory.getLogger();
    this.context = context;
    this.referrerCallback = referrerCallback;
    this.shouldTryToRead = new AtomicBoolean(true);
  }

  public void readReferrer() {
    if (!shouldTryToRead.get()) {
      logger.debug("Should not try to read Install referrer Huawei");
      return;
    }

    if (!Util.resolveContentProvider(context, REFERRER_PROVIDER_AUTHORITY)) {
      return;
    }

    Cursor cursor = null;
    Uri uri = Uri.parse(REFERRER_PROVIDER_URI);
    ContentResolver contentResolver = context.getContentResolver();

    String packageName[] = new String[] {context.getPackageName()};
    try {
      cursor = contentResolver.query(uri, null, null, packageName, null);

      if (cursor != null && cursor.moveToFirst()) {
        String installReferrer = cursor.getString(0);
        String clickTime = cursor.getString(1);
        String installTime = cursor.getString(2);

        logger.debug("InstallReferrerHuawei reads referrer[%s] clickTime[%s] installTime[%s]",
            installReferrer, clickTime, installTime);

        long referrerClickTimestampSeconds = Long.parseLong(clickTime);
        long installBeginTimestampSeconds = Long.parseLong(installTime);

        ReferrerDetails referrerDetails = new ReferrerDetails(
            installReferrer, referrerClickTimestampSeconds, installBeginTimestampSeconds);

        referrerCallback.onInstallReferrerRead(referrerDetails);

      } else {
        logger.debug(
            "InstallReferrerHuawei fail to read referrer for package [%s] and content uri [%s]",
            context.getPackageName(), uri.toString());
      }
    } catch (Exception e) {
      logger.debug("InstallReferrerHuawei error [%s]", e.getMessage());
    } finally {
      if (cursor != null) {
        cursor.close();
      }
    }

    shouldTryToRead.set(false);
  }
}
