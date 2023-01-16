/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.firefox.vpn.qt;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.Manifest.permission;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

// Gets used by /platforms/android/androidAppListProvider.cpp
public class PackageManagerHelper {
  final static String TAG = "PackageManagerHelper";

  private static String getAllAppNames(Context ctx) {
    JSONObject output = new JSONObject();
    PackageManager pm = ctx.getPackageManager();
    List<String> browsers = getBrowserIDs(pm);
    List<PackageInfo> packs = pm.getInstalledPackages(PackageManager.GET_PERMISSIONS);
    for (int i = 0; i < packs.size(); i++) {
      PackageInfo p = packs.get(i);
      // Do not add ourselves and System Apps to the list, unless it might be a browser
      if ((!isSystemPackage(p,pm) || browsers.contains(p.packageName))
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

  private static boolean isSystemPackage(PackageInfo pkgInfo, PackageManager pm) {
    if( (pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0){
      // no system app
      return false;
    }
    // For Systems Packages there are Cases where we want to add it anyway:
    // Has the use Internet permission (otherwise makes no sense)
    // Had at least 1 update (this means it's probably on any AppStore)
    // Has a a launch activity (has a ui and is not just a system service)

    if(!usesInternet(pkgInfo)){
      return true;
    }
    if(!hadUpdate(pkgInfo)){
      return true;
    }
    if(pm.getLaunchIntentForPackage(pkgInfo.packageName) == null){
      // If there is no way to launch this from a homescreen, def a sys package
      return true;
    }
    return false;
  }
  private static boolean isSelf(PackageInfo pkgInfo) {
    return pkgInfo.packageName.equals("org.mozilla.firefox.vpn")
        || pkgInfo.packageName.equals("org.mozilla.firefox.vpn.debug");
  }
  private static boolean usesInternet(PackageInfo pkgInfo){
    if(pkgInfo.requestedPermissions == null){
      return false;
    }
    for(int i=0; i < pkgInfo.requestedPermissions.length; i++) {
      String permission = pkgInfo.requestedPermissions[i];
      if(Manifest.permission.INTERNET.equals(permission)){
        return true;
      }
    }
    return false;
  }
  private static boolean hadUpdate(PackageInfo pkgInfo){
    return pkgInfo.lastUpdateTime > pkgInfo.firstInstallTime;
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
}
