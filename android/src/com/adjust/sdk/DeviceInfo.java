package com.adjust.sdk;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.util.DisplayMetrics;

import java.util.Date;
import java.util.Locale;

import static android.content.res.Configuration.UI_MODE_TYPE_MASK;
import static android.content.res.Configuration.UI_MODE_TYPE_TELEVISION;
import static com.adjust.sdk.Constants.HIGH;
import static com.adjust.sdk.Constants.LARGE;
import static com.adjust.sdk.Constants.LONG;
import static com.adjust.sdk.Constants.LOW;
import static com.adjust.sdk.Constants.MEDIUM;
import static com.adjust.sdk.Constants.NORMAL;
import static com.adjust.sdk.Constants.SMALL;
import static com.adjust.sdk.Constants.XLARGE;

/**
 * Created by pfms on 06/11/14.
 */
class DeviceInfo {
  private static final String OFFICIAL_FACEBOOK_SIGNATURE =
      "30820268308201d102044a9c4610300d06092a864886f70d0101040500307a310b3009060355040613"
      + "025553310b3009060355040813024341311230100603550407130950616c6f20416c746f31"
      + "183016060355040a130f46616365626f6f6b204d6f62696c653111300f060355040b130846"
      + "616365626f6f6b311d301b0603550403131446616365626f6f6b20436f72706f726174696f"
      + "6e3020170d3039303833313231353231365a180f32303530303932353231353231365a307a"
      + "310b3009060355040613025553310b30090603550408130243413112301006035504071309"
      + "50616c6f20416c746f31183016060355040a130f46616365626f6f6b204d6f62696c653111"
      + "300f060355040b130846616365626f6f6b311d301b0603550403131446616365626f6f6b20"
      + "436f72706f726174696f6e30819f300d06092a864886f70d010101050003818d0030818902"
      + "818100c207d51df8eb8c97d93ba0c8c1002c928fab00dc1b42fca5e66e99cc3023ed2d214d"
      + "822bc59e8e35ddcf5f44c7ae8ade50d7e0c434f500e6c131f4a2834f987fc46406115de201"
      + "8ebbb0d5a3c261bd97581ccfef76afc7135a6d59e8855ecd7eacc8f8737e794c60a761c536"
      + "b72b11fac8e603f5da1a2d54aa103b8a13c0dbc10203010001300d06092a864886f70d0101"
      + "040500038181005ee9be8bcbb250648d3b741290a82a1c9dc2e76a0af2f2228f1d9f9c4007"
      + "529c446a70175c5a900d5141812866db46be6559e2141616483998211f4a673149fb2232a1"
      + "0d247663b26a9031e15f84bc1c74d141ff98a02d76f85b2c8ab2571b6469b232d8e768a7f7"
      + "ca04f7abe4a775615916c07940656b58717457b42bd928a2";

  String playAdId;
  String playAdIdSource;
  int playAdIdAttempt;
  Boolean isTrackingEnabled;
  private boolean nonGoogleIdsReadOnce = false;
  String macSha1;
  String macShortMd5;
  String androidId;
  String fbAttributionId;
  String clientSdk;
  String packageName;
  String appVersion;
  String deviceType;
  String deviceName;
  String deviceManufacturer;
  String osName;
  String osVersion;
  String apiLevel;
  String language;
  String country;
  String screenSize;
  String screenFormat;
  String screenDensity;
  String displayWidth;
  String displayHeight;
  String hardwareName;
  String abi;
  String buildName;
  String appInstallTime;
  String appUpdateTime;
  int uiMode;

  DeviceInfo(Context context, String sdkPrefix) {
    Resources resources = context.getResources();
    DisplayMetrics displayMetrics = resources.getDisplayMetrics();
    Configuration configuration = resources.getConfiguration();
    Locale locale = Util.getLocale(configuration);
    int screenLayout = configuration.screenLayout;

    packageName = getPackageName(context);
    appVersion = getAppVersion(context);
    deviceType = getDeviceType(configuration);
    deviceName = getDeviceName();
    deviceManufacturer = getDeviceManufacturer();
    osName = getOsName();
    osVersion = getOsVersion();
    apiLevel = getApiLevel();
    language = getLanguage(locale);
    country = getCountry(locale);
    screenSize = getScreenSize(screenLayout);
    screenFormat = getScreenFormat(screenLayout);
    screenDensity = getScreenDensity(displayMetrics);
    displayWidth = getDisplayWidth(displayMetrics);
    displayHeight = getDisplayHeight(displayMetrics);
    clientSdk = getClientSdk(sdkPrefix);
    fbAttributionId = getFacebookAttributionId(context);
    hardwareName = getHardwareName();
    abi = getABI();
    buildName = getBuildName();
    appInstallTime = getAppInstallTime(context);
    appUpdateTime = getAppUpdateTime(context);
    uiMode = getDeviceUiMode(configuration);
  }

  void reloadPlayIds(Context context) {
    String previousPlayAdId = playAdId;
    Boolean previousIsTrackingEnabled = isTrackingEnabled;

    playAdId = null;
    isTrackingEnabled = null;
    playAdIdSource = null;
    playAdIdAttempt = -1;

    // attempt connecting to Google Play Service by own
    for (int serviceAttempt = 1; serviceAttempt <= 3; serviceAttempt += 1) {
      try {
        // timeout is a multiplier of the attempt number with 3 seconds
        // so first 3 seconds, second 6 seconds and third and last 9 seconds
        long timeoutServiceMilli = Constants.ONE_SECOND * 3 * serviceAttempt;
        GooglePlayServicesClient.GooglePlayServicesInfo gpsInfo =
            GooglePlayServicesClient.getGooglePlayServicesInfo(context, timeoutServiceMilli);
        if (playAdId == null) {
          playAdId = gpsInfo.getGpsAdid();
        }
        if (isTrackingEnabled == null) {
          isTrackingEnabled = gpsInfo.isTrackingEnabled();
        }

        if (playAdId != null && isTrackingEnabled != null) {
          playAdIdSource = "service";
          playAdIdAttempt = serviceAttempt;
          return;
        }
      } catch (Exception e) {
      }
    }

    // as fallback attempt connecting to Google Play Service using library
    for (int libAttempt = 1; libAttempt <= 3; libAttempt += 1) {
      // timeout inside library is 10 seconds, so 10 + 1 seconds are given
      Object advertisingInfoObject =
          Util.getAdvertisingInfoObject(context, Constants.ONE_SECOND * 11);

      if (advertisingInfoObject == null) {
        continue;
      }

      if (playAdId == null) {
        // just needs a short timeout since it should be just accessing a POJO
        playAdId = Util.getPlayAdId(context, advertisingInfoObject, Constants.ONE_SECOND);
      }
      if (isTrackingEnabled == null) {
        // just needs a short timeout since it should be just accessing a POJO
        isTrackingEnabled =
            Util.isPlayTrackingEnabled(context, advertisingInfoObject, Constants.ONE_SECOND);
      }

      if (playAdId != null && isTrackingEnabled != null) {
        playAdIdSource = "library";
        playAdIdAttempt = libAttempt;
        return;
      }
    }

    // if both weren't found, use previous values
    if (playAdId == null) {
      playAdId = previousPlayAdId;
    }
    if (isTrackingEnabled == null) {
      isTrackingEnabled = previousIsTrackingEnabled;
    }
  }

  void reloadNonPlayIds(Context context) {
    if (nonGoogleIdsReadOnce) {
      return;
    }
    if (!Util.checkPermission(context, android.Manifest.permission.ACCESS_WIFI_STATE)) {
      AdjustFactory.getLogger().warn("Missing permission: ACCESS_WIFI_STATE");
    }
    String macAddress = Util.getMacAddress(context);
    macSha1 = getMacSha1(macAddress);
    macShortMd5 = getMacShortMd5(macAddress);
    androidId = Util.getAndroidId(context);
    nonGoogleIdsReadOnce = true;
  }

  private String getMacAddress(Context context, boolean isGooglePlayServicesAvailable) {
    if (!isGooglePlayServicesAvailable) {
      if (!Util.checkPermission(context, android.Manifest.permission.ACCESS_WIFI_STATE)) {
        AdjustFactory.getLogger().warn("Missing permission: ACCESS_WIFI_STATE");
      }
      return Util.getMacAddress(context);
    } else {
      return null;
    }
  }

  private String getPackageName(Context context) {
    return context.getPackageName();
  }

  private String getAppVersion(Context context) {
    try {
      PackageManager packageManager = context.getPackageManager();
      String name = context.getPackageName();
      PackageInfo info = packageManager.getPackageInfo(name, 0);
      return info.versionName;
    } catch (Exception e) {
      return null;
    }
  }

  private String getDeviceType(Configuration configuration) {
    int uiMode = configuration.uiMode & UI_MODE_TYPE_MASK;
    if (uiMode == UI_MODE_TYPE_TELEVISION) {
      return "tv";
    }

    int screenSize = configuration.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK;
    switch (screenSize) {
      case Configuration.SCREENLAYOUT_SIZE_SMALL:
      case Configuration.SCREENLAYOUT_SIZE_NORMAL:
        return "phone";
      case Configuration.SCREENLAYOUT_SIZE_LARGE:
      case 4:
        return "tablet";
      default:
        return null;
    }
  }

  private int getDeviceUiMode(Configuration configuration) {
    return configuration.uiMode & UI_MODE_TYPE_MASK;
  }

  private String getDeviceName() {
    return Build.MODEL;
  }

  private String getDeviceManufacturer() {
    return Build.MANUFACTURER;
  }

  private String getOsName() {
    return "android";
  }

  private String getOsVersion() {
    return Build.VERSION.RELEASE;
  }

  private String getApiLevel() {
    return "" + Build.VERSION.SDK_INT;
  }

  private String getLanguage(Locale locale) {
    return locale.getLanguage();
  }

  private String getCountry(Locale locale) {
    return locale.getCountry();
  }

  private String getBuildName() {
    return Build.ID;
  }

  private String getHardwareName() {
    return Build.DISPLAY;
  }

  private String getScreenSize(int screenLayout) {
    int screenSize = screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK;

    switch (screenSize) {
      case Configuration.SCREENLAYOUT_SIZE_SMALL:
        return SMALL;
      case Configuration.SCREENLAYOUT_SIZE_NORMAL:
        return NORMAL;
      case Configuration.SCREENLAYOUT_SIZE_LARGE:
        return LARGE;
      case 4:
        return XLARGE;
      default:
        return null;
    }
  }

  private String getScreenFormat(int screenLayout) {
    int screenFormat = screenLayout & Configuration.SCREENLAYOUT_LONG_MASK;

    switch (screenFormat) {
      case Configuration.SCREENLAYOUT_LONG_YES:
        return LONG;
      case Configuration.SCREENLAYOUT_LONG_NO:
        return NORMAL;
      default:
        return null;
    }
  }

  private String getScreenDensity(DisplayMetrics displayMetrics) {
    int density = displayMetrics.densityDpi;
    int low = (DisplayMetrics.DENSITY_MEDIUM + DisplayMetrics.DENSITY_LOW) / 2;
    int high = (DisplayMetrics.DENSITY_MEDIUM + DisplayMetrics.DENSITY_HIGH) / 2;

    if (density == 0) {
      return null;
    } else if (density < low) {
      return LOW;
    } else if (density > high) {
      return HIGH;
    }
    return MEDIUM;
  }

  private String getDisplayWidth(DisplayMetrics displayMetrics) {
    return String.valueOf(displayMetrics.widthPixels);
  }

  private String getDisplayHeight(DisplayMetrics displayMetrics) {
    return String.valueOf(displayMetrics.heightPixels);
  }

  private String getClientSdk(String sdkPrefix) {
    if (sdkPrefix == null) {
      return Constants.CLIENT_SDK;
    } else {
      return Util.formatString("%s@%s", sdkPrefix, Constants.CLIENT_SDK);
    }
  }

  private String getMacSha1(String macAddress) {
    if (macAddress == null) {
      return null;
    }
    String macSha1 = Util.sha1(macAddress);

    return macSha1;
  }

  private String getMacShortMd5(String macAddress) {
    if (macAddress == null) {
      return null;
    }
    String macShort = macAddress.replaceAll(":", "");
    String macShortMd5 = Util.md5(macShort);

    return macShortMd5;
  }

  private String getFacebookAttributionId(final Context context) {
    try {
      @SuppressLint("PackageManagerGetSignatures")
      Signature[] signatures =
          context.getPackageManager()
              .getPackageInfo("com.facebook.katana", PackageManager.GET_SIGNATURES)
              .signatures;
      if (signatures == null || signatures.length != 1) {
        // Unable to find the correct signatures for this APK
        return null;
      }
      Signature facebookApkSignature = signatures[0];
      if (!OFFICIAL_FACEBOOK_SIGNATURE.equals(facebookApkSignature.toCharsString())) {
        // not the official Facebook application
        return null;
      }

      final ContentResolver contentResolver = context.getContentResolver();
      final Uri uri = Uri.parse("content://com.facebook.katana.provider.AttributionIdProvider");
      final String columnName = "aid";
      final String[] projection = {columnName};
      final Cursor cursor = contentResolver.query(uri, projection, null, null, null);

      if (cursor == null) {
        return null;
      }
      if (!cursor.moveToFirst()) {
        cursor.close();
        return null;
      }

      final String attributionId = cursor.getString(cursor.getColumnIndex(columnName));
      cursor.close();
      return attributionId;
    } catch (Exception e) {
      return null;
    }
  }

  private String getABI() {
    String[] SupportedABIS = Util.getSupportedAbis();

    // SUPPORTED_ABIS is only supported in API level 21
    // get CPU_ABI instead
    if (SupportedABIS == null || SupportedABIS.length == 0) {
      return Util.getCpuAbi();
    }

    return SupportedABIS[0];
  }

  private String getAppInstallTime(Context context) {
    try {
      PackageManager packageManager = context.getPackageManager();
      PackageInfo packageInfo =
          packageManager.getPackageInfo(context.getPackageName(), PackageManager.GET_PERMISSIONS);

      String appInstallTime = Util.dateFormatter.format(new Date(packageInfo.firstInstallTime));

      return appInstallTime;
    } catch (Exception ex) {
      return null;
    }
  }

  private String getAppUpdateTime(Context context) {
    try {
      PackageManager packageManager = context.getPackageManager();
      PackageInfo packageInfo =
          packageManager.getPackageInfo(context.getPackageName(), PackageManager.GET_PERMISSIONS);

      String appInstallTime = Util.dateFormatter.format(new Date(packageInfo.lastUpdateTime));

      return appInstallTime;
    } catch (Exception ex) {
      return null;
    }
  }
}
