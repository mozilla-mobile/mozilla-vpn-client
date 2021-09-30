package com.adjust.sdk;

import android.content.Context;
import android.net.Uri;

import org.json.JSONObject;

/**
 * The main interface to Adjust.
 * Use the methods of this class to tell Adjust about the usage of your app.
 * See the README for details.
 *
 * @author Christian Wellenbrock (@wellle)
 * @since 11th November 2011
 */
public class Adjust {
  /**
   * Singleton Adjust SDK instance.
   */
  private static AdjustInstance defaultInstance;

  /**
   * Private constructor.
   */
  private Adjust() {}

  /**
   * Method used to obtain Adjust SDK singleton instance.
   *
   * @return Adjust SDK singleton instance.
   */
  public static synchronized AdjustInstance getDefaultInstance() {
    @SuppressWarnings("unused")
    String VERSION = "!SDK-VERSION-STRING!:com.adjust.sdk:adjust-android:4.28.5";

    if (defaultInstance == null) {
      defaultInstance = new AdjustInstance();
    }
    return defaultInstance;
  }

  /**
   * Called upon SDK initialisation.
   *
   * @param adjustConfig AdjustConfig object used for SDK initialisation
   */
  public static void onCreate(AdjustConfig adjustConfig) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.onCreate(adjustConfig);
  }

  /**
   * Called to track event.
   *
   * @param event AdjustEvent object to be tracked
   */
  public static void trackEvent(AdjustEvent event) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackEvent(event);
  }

  /**
   * Called upon each Activity's onResume() method call.
   */
  public static void onResume() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.onResume();
  }

  /**
   * Called upon each Activity's onPause() method call.
   */
  public static void onPause() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.onPause();
  }

  /**
   * Called to disable/enable SDK.
   *
   * @param enabled boolean indicating whether SDK should be enabled or disabled
   */
  public static void setEnabled(boolean enabled) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.setEnabled(enabled);
  }

  /**
   * Get information if SDK is enabled or not.
   *
   * @return boolean indicating whether SDK is enabled or not
   */
  public static boolean isEnabled() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    return adjustInstance.isEnabled();
  }

  /**
   * Called to process deep link.
   *
   * @param url Deep link URL to process
   *
   * @deprecated Use {@link #appWillOpenUrl(Uri, Context)}} instead.
   */
  @Deprecated
  public static void appWillOpenUrl(Uri url) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.appWillOpenUrl(url);
  }

  /**
   * Called to process deep link.
   *
   * @param url Deep link URL to process
   * @param context Application context
   */
  public static void appWillOpenUrl(Uri url, Context context) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.appWillOpenUrl(url, context);
  }

  /**
   * Called to process referrer information sent with INSTALL_REFERRER intent.
   *
   * @param referrer Referrer content
   * @param context  Application context
   */
  public static void setReferrer(String referrer, Context context) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.sendReferrer(referrer, context);
  }

  /**
   * Called to set SDK to offline or online mode.
   *
   * @param enabled boolean indicating should SDK be in offline mode (true) or not (false)
   */
  public static void setOfflineMode(boolean enabled) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.setOfflineMode(enabled);
  }

  /**
   * Called if SDK initialisation was delayed and you would like to stop waiting for timer.
   */
  public static void sendFirstPackages() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.sendFirstPackages();
  }

  /**
   * Called to add global callback parameter that will be sent with each session and event.
   *
   * @param key   Global callback parameter key
   * @param value Global callback parameter value
   */
  public static void addSessionCallbackParameter(String key, String value) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.addSessionCallbackParameter(key, value);
  }

  /**
   * Called to add global partner parameter that will be sent with each session and event.
   *
   * @param key   Global partner parameter key
   * @param value Global partner parameter value
   */
  public static void addSessionPartnerParameter(String key, String value) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.addSessionPartnerParameter(key, value);
  }

  /**
   * Called to remove global callback parameter from session and event packages.
   *
   * @param key Global callback parameter key
   */
  public static void removeSessionCallbackParameter(String key) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.removeSessionCallbackParameter(key);
  }

  /**
   * Called to remove global partner parameter from session and event packages.
   *
   * @param key Global partner parameter key
   */
  public static void removeSessionPartnerParameter(String key) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.removeSessionPartnerParameter(key);
  }

  /**
   * Called to remove all added global callback parameters.
   */
  public static void resetSessionCallbackParameters() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.resetSessionCallbackParameters();
  }

  /**
   * Called to remove all added global partner parameters.
   */
  public static void resetSessionPartnerParameters() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.resetSessionPartnerParameters();
  }

  /**
   * Called to set user's push notifications token.
   *
   * @param token Push notifications token
   * @deprecated use {@link #setPushToken(String, Context)} instead.
   */
  public static void setPushToken(String token) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.setPushToken(token);
  }

  /**
   * Called to set user's push notifications token.
   *
   * @param token   Push notifications token
   * @param context Application context
   */
  public static void setPushToken(final String token, final Context context) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.setPushToken(token, context);
  }

  /**
   * Called to forget the user in accordance with GDPR law.
   *
   * @param context Application context
   */
  public static void gdprForgetMe(final Context context) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.gdprForgetMe(context);
  }

  /**
   * Called to disable the third party sharing.
   *
   * @param context Application context
   */
  public static void disableThirdPartySharing(final Context context) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.disableThirdPartySharing(context);
  }

  public static void trackThirdPartySharing(final AdjustThirdPartySharing adjustThirdPartySharing) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackThirdPartySharing(adjustThirdPartySharing);
  }

  public static void trackMeasurementConsent(final boolean consentMeasurement) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackMeasurementConsent(consentMeasurement);
  }

  /**
   * Track ad revenue from a source provider
   *
   * @param source Source of ad revenue information, see AdjustConfig.AD_REVENUE_* for some possible
   *     sources
   * @param payload JsonObject content of the ad revenue information
   */
  public static void trackAdRevenue(final String source, final JSONObject payload) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackAdRevenue(source, payload);
  }

  /**
   * Track ad revenue from a source provider
   *
   * @param adjustAdRevenue Adjust ad revenue information like source, revenue, currency etc
   */
  public static void trackAdRevenue(final AdjustAdRevenue adjustAdRevenue) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackAdRevenue(adjustAdRevenue);
  }

  /**
   * Track subscription from Google Play.
   *
   * @param subscription AdjustPlayStoreSubscription object to be tracked
   */
  public static void trackPlayStoreSubscription(final AdjustPlayStoreSubscription subscription) {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.trackPlayStoreSubscription(subscription);
  }

  /**
   * Called to get value of Google Play Advertising Identifier.
   *
   * @param context        Application context
   * @param onDeviceIdRead Callback to get triggered once identifier is obtained
   */
  public static void getGoogleAdId(Context context, OnDeviceIdsRead onDeviceIdRead) {
    Util.getGoogleAdId(context, onDeviceIdRead);
  }

  /**
   * Called to get value of Amazon Advertising Identifier.
   *
   * @param context Application context
   * @return Amazon Advertising Identifier
   */
  public static String getAmazonAdId(final Context context) {
    return Util.getFireAdvertisingId(context.getContentResolver());
  }

  /**
   * Called to get value of unique Adjust device identifier.
   *
   * @return Unique Adjust device indetifier
   */
  public static String getAdid() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    return adjustInstance.getAdid();
  }

  /**
   * Called to get user's current attribution value.
   *
   * @return AdjustAttribution object with current attribution value
   */
  public static AdjustAttribution getAttribution() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    return adjustInstance.getAttribution();
  }

  /**
   * Called to get native SDK version string.
   *
   * @return Native SDK version string.
   */
  public static String getSdkVersion() {
    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    return adjustInstance.getSdkVersion();
  }

  /**
   * Used for testing purposes only. Do NOT use this method.
   *
   * @param testOptions Adjust integration tests options
   */
  public static void setTestOptions(AdjustTestOptions testOptions) {
    if (testOptions.teardown != null && testOptions.teardown.booleanValue()) {
      if (defaultInstance != null) {
        defaultInstance.teardown();
      }
      defaultInstance = null;
      AdjustFactory.teardown(testOptions.context);
    }

    AdjustInstance adjustInstance = Adjust.getDefaultInstance();
    adjustInstance.setTestOptions(testOptions);
  }
}
