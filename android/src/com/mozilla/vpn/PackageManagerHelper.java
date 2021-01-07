/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package com.mozilla.vpn;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.UserHandle;
import android.os.UserManager;
import android.util.Log;
import android.webkit.WebView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Pattern;

// Gets used by /platforms/android/androidAppListProvider.cpp
public class PackageManagerHelper {
  final static String TAG = "PackageManagerHelper";
  final static int MIN_CHROME_VERSION = 65;

  // These system apps will not be hidden to the user (if installed):
  final static List<String> SYSTEM_ALLOWLIST = Arrays.asList(new String[] {
      "com.android.vending", // Google Play Store
      "com.google.android.apps.chromecast.app", // Google Home
      "com.google.android.apps.maps", // Google Maps
      "com.google.android.apps.walletnfcrel", // Google Pay
      "com.google.android.calendar", // Gcal
      "com.google.android.gm", // Gmail
      "com.google.android.music", // Gmusic
      "com.google.android.videos", // Play video
      "com.google.android.youtube", // Youtube
      "com.google.android.projection.gearhead", // Android Auto
      "com.google.android.apps.magazines", // Google news
      "com.google.android.GoogleCamera", // Google Camera
      "com.android.hotwordenrollment.xgoogle", // Google Assistant
      "com.android.hotwordenrollment.okgoogle", // Google Assistant
      "com.google.android.gms.location.history", // Google Location History
  });
  final static List<String> CHROME_BROWSERS = Arrays.asList(
      new String[] {"com.google.android.webview", "com.android.webview", "com.google.chrome"});

  private static String getAllAppNames(Context ctx) {
    JSONObject output = new JSONObject();
    PackageManager pm = ctx.getPackageManager();
    List<String> browsers = getBrowserIDs(pm);
    List<PackageInfo> packs = pm.getInstalledPackages(0);
    for (int i = 0; i < packs.size(); i++) {
      PackageInfo p = packs.get(i);
      // Do not add ourselves and System Apps to the list, unless it might be a browser
      if ((!isSystemPackage(p) || browsers.contains(p.packageName)
              || SYSTEM_ALLOWLIST.contains(p.packageName))
          && !isSelf(p)) {
        String appid = p.packageName;
        String appName = p.applicationInfo.loadLabel(pm).toString();
        try {
          output.put(appid, appName);
        } catch (JSONException e) {
          e.printStackTrace();
        }
      }
    }
    return output.toString();
  }

  private static Drawable getAppIcon(Context ctx, String id) {
    try {
      return ctx.getPackageManager().getApplicationIcon(id);
    } catch (PackageManager.NameNotFoundException e) {
      e.printStackTrace();
    }
    return new ColorDrawable(Color.TRANSPARENT);
  }

  private static boolean isSystemPackage(PackageInfo pkgInfo) {
    return (pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0;
  }
  private static boolean isSelf(PackageInfo pkgInfo) {
    return pkgInfo.packageName.equals("org.mozilla.firefox.vpn")
        || pkgInfo.packageName.equals("org.mozilla.firefox.vpn.debug");
  }

  // Returns List of all Packages that can classify themselves as browsers
  private static List<String> getBrowserIDs(PackageManager pm) {
    Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://www.mozilla.org/"));
    intent.addCategory(Intent.CATEGORY_BROWSABLE);
    // We've tried using PackageManager.MATCH_DEFAULT_ONLY flag and found that browsers that
    // are not set as the default browser won't be matched even if they had CATEGORY_DEFAULT set
    // in the intent filter

    List<ResolveInfo> resolveInfos = pm.queryIntentActivities(intent, PackageManager.MATCH_ALL);
    List<String> browsers = new ArrayList<String>();
    for (int i = 0; i < resolveInfos.size(); i++) {
      ResolveInfo info = resolveInfos.get(i);
      String browserID = info.activityInfo.packageName;
      browsers.add(browserID);
    }
    return browsers;
  }

  // Gets called in AndroidAuthenticationListener;
  public static boolean isWebViewSupported(Context ctx) {
    Log.v(TAG, "Checking if installed Webview is compatible with FxA");
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
      // The default Webview is able do to FXA
      return true;
    }
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      PackageInfo pi = WebView.getCurrentWebViewPackage();
      if (CHROME_BROWSERS.contains(pi.packageName)) {
        return isSupportedChromeBrowser(pi);
      }
      return isNotAncientBrowser(pi);
    }

    // Before O the webview is hardcoded, but we dont know which package it is.
    // Check if com.google.android.webview is installed
    PackageManager pm = ctx.getPackageManager();
    try {
      PackageInfo pi = pm.getPackageInfo("com.google.android.webview", 0);
      return isSupportedChromeBrowser(pi);
    } catch (PackageManager.NameNotFoundException e) {
    }
    // Otherwise check com.android.webview
    try {
      PackageInfo pi = pm.getPackageInfo("com.android.webview", 0);
      return isSupportedChromeBrowser(pi);
    } catch (PackageManager.NameNotFoundException e) {
    }
    Log.e(TAG, "Android System WebView is not found");
    // Giving up :(
    return false;
  }

  private static boolean isSupportedChromeBrowser(PackageInfo pi) {
    Log.d(TAG, "Checking Chrome Based Browser: " + pi.packageName);
    Log.d(TAG, "version name: " + pi.versionName);
    Log.d(TAG, "version code: " + pi.versionCode);
    try {
      String versionCode = pi.versionName.split(Pattern.quote(" "))[0];
      String majorVersion = versionCode.split(Pattern.quote("."))[0];
      int version = Integer.parseInt(majorVersion);
      return version >= MIN_CHROME_VERSION;
    } catch (Exception e) {
      Log.e(TAG, "Failed to check Chrome Version Code " + pi.versionName);
      return false;
    }
  }

  private static boolean isNotAncientBrowser(PackageInfo pi) {
    // Not a google chrome - So the version name is worthless
    // Lets just make sure the WebView
    // used is not ancient ==> Was updated in at least the last 365 days
    Log.d(TAG, "Checking Chrome Based Browser: " + pi.packageName);
    Log.d(TAG, "version name: " + pi.versionName);
    Log.d(TAG, "version code: " + pi.versionCode);
    double oneYearInMillis = 31536000000L;
    return pi.lastUpdateTime > (System.currentTimeMillis() - oneYearInMillis);
  }
}
