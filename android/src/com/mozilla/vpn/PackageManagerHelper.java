package com.mozilla.vpn;

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
import android.os.UserHandle;
import android.os.UserManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

// Gets used by /platforms/android/androidAppListProvider.cpp
public class PackageManagerHelper {
  private static String getAllAppNames(Context ctx) {
    JSONObject output = new JSONObject();
    PackageManager pm = ctx.getPackageManager();
    List<String> browsers = getBrowserIDs(pm);
    List<PackageInfo> packs = pm.getInstalledPackages(0);
    for (int i = 0; i < packs.size(); i++) {
      PackageInfo p = packs.get(i);
      // Do not add ourselves and System Apps to the list, unless it might be a browser
      if ((!isSystemPackage(p) || browsers.contains(p.packageName)) && !isSelf(p)) {
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
}
