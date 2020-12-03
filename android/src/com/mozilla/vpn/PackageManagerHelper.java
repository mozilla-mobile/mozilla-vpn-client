package com.mozilla.vpn;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.UserHandle;
import android.os.UserManager;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;

// Gets used by /platforms/android/androidAppListProvider.cpp
public class PackageManagerHelper {
  private static String getAllAppNames(Context ctx) {
    JSONObject output = new JSONObject();
    List<PackageInfo> packs = ctx.getPackageManager().getInstalledPackages(0);
    for (int i = 0; i < packs.size(); i++) {
      PackageInfo p = packs.get(i);
      if ((!isSystemPackage(p))) {
        String appid = p.packageName;
        String appName = p.applicationInfo.loadLabel(ctx.getPackageManager()).toString();
        try {
          output.put(appid, appName);
        } catch (JSONException e) {
          e.printStackTrace();
        }
      }
    }
    return output.toString();
  }

  private static boolean isSystemPackage(PackageInfo pkgInfo) {
    return (pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0;
  }
}
