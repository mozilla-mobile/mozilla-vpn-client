package com.adjust.sdk;

import org.json.JSONArray;
import org.json.JSONException;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;

/**
 * Class used for shared preferences manipulation.
 *
 * @author Ugljesa Erceg (uerceg)
 * @since 7th July 2017
 */

public class SharedPreferencesManager {
  /**
   * Name of Adjust preferences.
   */
  private static final String PREFS_NAME = "adjust_preferences";

  /**
   * Key name for referrers.
   */
  private static final String PREFS_KEY_RAW_REFERRERS = "raw_referrers";

  /**
   * Key name for push token.
   */
  private static final String PREFS_KEY_PUSH_TOKEN = "push_token";

  /**
   * Key name for info about whether install has been tracked or not.
   */
  private static final String PREFS_KEY_INSTALL_TRACKED = "install_tracked";

  private static final String PREFS_KEY_GDPR_FORGET_ME = "gdpr_forget_me";

  private static final String PREFS_KEY_DISABLE_THIRD_PARTY_SHARING = "disable_third_party_sharing";

  private static final String PREFS_KEY_DEEPLINK_URL = "deeplink_url";

  private static final String PREFS_KEY_DEEPLINK_CLICK_TIME = "deeplink_click_time";

  private static final String PREFS_KEY_PREINSTALL_PAYLOAD_READ_STATUS =
      "preinstall_payload_read_status";

  private static final String PREFS_KEY_PREINSTALL_SYSTEM_INSTALLER_REFERRER =
      "preinstall_system_installer_referrer";

  /**
   * Index for raw referrer string content in saved JSONArray object.
   */
  private static final int INDEX_RAW_REFERRER = 0;

  /**
   * Index for click time in saved JSONArray object.
   */
  private static final int INDEX_CLICK_TIME = 1;

  /**
   * Index for information whether referrer is being sent in saved JSONArray object.
   */
  private static final int INDEX_IS_SENDING = 2;

  /**
   * Number of persisted referrers.
   */
  private static final int REFERRERS_COUNT = 10;

  /**
   * Shared preferences of the app.
   */
  private final SharedPreferences sharedPreferences;

  /**
   * Default constructor.
   *
   * @param context Application context
   */
  public SharedPreferencesManager(final Context context) {
    this.sharedPreferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
  }

  /**
   * Save raw referrer string into shared preferences.
   *
   * @param rawReferrer Raw referrer string
   * @param clickTime   Click time
   */
  public synchronized void saveRawReferrer(final String rawReferrer, final long clickTime) {
    try {
      if (getRawReferrer(rawReferrer, clickTime) != null) {
        return;
      }

      JSONArray rawReferrerArray = getRawReferrerArray();

      // There are exactly REFERRERS_COUNT saved referrers, do nothing.
      if (rawReferrerArray.length() == REFERRERS_COUNT) {
        return;
      }

      JSONArray newRawReferrer = new JSONArray();
      newRawReferrer.put(INDEX_RAW_REFERRER, rawReferrer);
      newRawReferrer.put(INDEX_CLICK_TIME, clickTime);
      newRawReferrer.put(INDEX_IS_SENDING, 0);

      rawReferrerArray.put(newRawReferrer);
      saveRawReferrerArray(rawReferrerArray);
    } catch (JSONException e) {
    }
  }

  /**
   * Save referrer array to shared preferences.
   *
   * @param rawReferrerArray Array of referrers to be saved
   */
  public synchronized void saveRawReferrerArray(final JSONArray rawReferrerArray) {
    try {
      saveString(PREFS_KEY_RAW_REFERRERS, rawReferrerArray.toString());
    } catch (Throwable t) {
      remove(PREFS_KEY_RAW_REFERRERS);
    }
  }

  /**
   * Remove referrer information from shared preferences.
   *
   * @param clickTime   Click time
   * @param rawReferrer Raw referrer string
   */
  public synchronized void removeRawReferrer(final String rawReferrer, final long clickTime) {
    // Don't even try to remove null or empty referrers since they shouldn't exist in shared
    // preferences.
    if (rawReferrer == null || rawReferrer.length() == 0) {
      return;
    }

    int rawReferrerIndex = getRawReferrerIndex(rawReferrer, clickTime);

    if (rawReferrerIndex < 0) {
      return;
    }

    JSONArray rawReferrerArray = getRawReferrerArray();

    // Rebuild queue without referrer that should be removed.
    JSONArray updatedReferrers = new JSONArray();

    for (int i = 0; i < rawReferrerArray.length(); i += 1) {
      if (i == rawReferrerIndex) {
        continue;
      }

      try {
        updatedReferrers.put(rawReferrerArray.getJSONArray(i));
      } catch (JSONException e) {
      }
    }

    // Save new referrer queue JSON array as string back to shared preferences.
    saveString(PREFS_KEY_RAW_REFERRERS, updatedReferrers.toString());
  }

  /**
   * Get saved referrer JSONArray object.
   *
   * @param rawReferrer Raw referrer string
   * @param clickTime   Click time
   * @return JSONArray object containing referrer information. Defaults to null if not found.
   */
  public synchronized JSONArray getRawReferrer(final String rawReferrer, final long clickTime) {
    int rawReferrerIndex = getRawReferrerIndex(rawReferrer, clickTime);
    if (rawReferrerIndex >= 0) {
      try {
        return getRawReferrerArray().getJSONArray(rawReferrerIndex);
      } catch (JSONException e) {
      }
    }
    return null;
  }

  /**
   * Get array of saved referrer JSONArray objects.
   *
   * @return JSONArray of saved referrers. Defaults to empty JSONArray if none found.
   */
  public synchronized JSONArray getRawReferrerArray() {
    String referrerQueueString = getString(PREFS_KEY_RAW_REFERRERS);

    if (referrerQueueString != null) {
      try {
        JSONArray rawReferrerArray = new JSONArray(referrerQueueString);

        // Initial move for those who have more than REFERRERS_COUNT stored already.
        // Cut the array and leave it with only REFERRERS_COUNT elements.
        if (rawReferrerArray.length() > REFERRERS_COUNT) {
          JSONArray tempReferrerArray = new JSONArray();
          for (int i = 0; i < REFERRERS_COUNT; i += 1) {
            tempReferrerArray.put(rawReferrerArray.get(i));
          }
          saveRawReferrerArray(tempReferrerArray);
          return tempReferrerArray;
        }

        return new JSONArray(referrerQueueString);
      } catch (JSONException e) {
      } catch (Throwable t) {
      }
    }

    return new JSONArray();
  }

  /**
   * Save preinstall referrer string into shared preferences.
   *
   * @param referrer Preinstall referrer string
   */
  public synchronized void savePreinstallReferrer(final String referrer) {
    saveString(PREFS_KEY_PREINSTALL_SYSTEM_INSTALLER_REFERRER, referrer);
  }

  /**
   * Get saved preinstall referrer string from shared preferences.
   *
   * @return referrer Preinstall referrer string
   */
  public synchronized String getPreinstallReferrer() {
    return getString(PREFS_KEY_PREINSTALL_SYSTEM_INSTALLER_REFERRER);
  }

  /**
   * Remove saved preinstall referrer string from shared preferences.
   */
  public synchronized void removePreinstallReferrer() {
    remove(PREFS_KEY_PREINSTALL_SYSTEM_INSTALLER_REFERRER);
  }

  /**
   * Initially called upon ActivityHandler initialisation.
   * Used to check if any of the still existing referrers was unsuccessfully being sent before app
   * got killed. If such found - switch it's isBeingSent flag back to "false".
   */
  public synchronized void setSendingReferrersAsNotSent() {
    try {
      JSONArray rawReferrerArray = getRawReferrerArray();
      boolean hasRawReferrersBeenChanged = false;
      for (int i = 0; i < rawReferrerArray.length(); i++) {
        JSONArray rawReferrer = rawReferrerArray.getJSONArray(i);
        int sendingStatus = rawReferrer.optInt(INDEX_IS_SENDING, -1);
        if (sendingStatus == 1) {
          rawReferrer.put(INDEX_IS_SENDING, 0);
          hasRawReferrersBeenChanged = true;
        }
      }
      if (hasRawReferrersBeenChanged) {
        saveRawReferrerArray(rawReferrerArray);
      }
    } catch (JSONException e) {
    }
  }

  /**
   * Get index of saved raw referrer.
   *
   * @param rawReferrer Raw referrer string
   * @param clickTime   Click time
   * @return Index of saved referrer. Defaults to -1 if referrer not found.
   */
  private synchronized int getRawReferrerIndex(final String rawReferrer, final long clickTime) {
    try {
      JSONArray rawReferrers = getRawReferrerArray();

      for (int i = 0; i < rawReferrers.length(); i++) {
        JSONArray savedRawReferrer = rawReferrers.getJSONArray(i);
        // Check if raw referrer is already saved.
        String savedRawReferrerString = savedRawReferrer.optString(INDEX_RAW_REFERRER, null);
        if (savedRawReferrerString == null || !savedRawReferrerString.equals(rawReferrer)) {
          continue;
        }
        long savedClickTime = savedRawReferrer.optLong(INDEX_CLICK_TIME, -1);
        if (savedClickTime != clickTime) {
          continue;
        }
        // Install referrer found, skip adding it.
        return i;
      }
    } catch (JSONException e) {
    }

    return -1;
  }

  /**
   * Save push token to shared preferences.
   *
   * @param pushToken Push notifications token
   */
  public synchronized void savePushToken(final String pushToken) {
    saveString(PREFS_KEY_PUSH_TOKEN, pushToken);
  }

  /**
   * Get push token from shared preferences.
   *
   * @return Push token value
   */
  public synchronized String getPushToken() {
    return getString(PREFS_KEY_PUSH_TOKEN);
  }

  /**
   * Remove push token from shared preferences.
   */
  public synchronized void removePushToken() {
    remove(PREFS_KEY_PUSH_TOKEN);
  }

  /**
   * Save information that install has been tracked to shared preferences.
   */
  public synchronized void setInstallTracked() {
    saveBoolean(PREFS_KEY_INSTALL_TRACKED, true);
  }

  /**
   * Get information if install has been tracked from shared preferences. If no info, default to
   * false.
   *
   * @return boolean indicating whether install has been tracked or not
   */
  public synchronized boolean getInstallTracked() {
    return getBoolean(PREFS_KEY_INSTALL_TRACKED, false);
  }

  public synchronized void setGdprForgetMe() {
    saveBoolean(PREFS_KEY_GDPR_FORGET_ME, true);
  }

  public synchronized boolean getGdprForgetMe() {
    return getBoolean(PREFS_KEY_GDPR_FORGET_ME, false);
  }

  public synchronized void removeGdprForgetMe() {
    remove(PREFS_KEY_GDPR_FORGET_ME);
  }

  public synchronized void setDisableThirdPartySharing() {
    saveBoolean(PREFS_KEY_DISABLE_THIRD_PARTY_SHARING, true);
  }

  public synchronized boolean getDisableThirdPartySharing() {
    return getBoolean(PREFS_KEY_DISABLE_THIRD_PARTY_SHARING, false);
  }

  public synchronized void removeDisableThirdPartySharing() {
    remove(PREFS_KEY_DISABLE_THIRD_PARTY_SHARING);
  }

  public synchronized void saveDeeplink(final Uri deeplink, final long clickTime) {
    if (deeplink == null) {
      return;
    }

    saveString(PREFS_KEY_DEEPLINK_URL, deeplink.toString());
    saveLong(PREFS_KEY_DEEPLINK_CLICK_TIME, clickTime);
  }

  public synchronized String getDeeplinkUrl() {
    return getString(PREFS_KEY_DEEPLINK_URL);
  }

  public synchronized long getDeeplinkClickTime() {
    return getLong(PREFS_KEY_DEEPLINK_CLICK_TIME, -1);
  }

  public synchronized void removeDeeplink() {
    remove(PREFS_KEY_DEEPLINK_URL);
    remove(PREFS_KEY_DEEPLINK_CLICK_TIME);
  }

  /**
   * Save information that preinstall tracker has been tracked to shared preferences.
   */
  public synchronized void setPreinstallPayloadReadStatus(long status) {
    saveLong(PREFS_KEY_PREINSTALL_PAYLOAD_READ_STATUS, status);
  }

  /**
   * Get information if preinstall tracker has been tracked from shared preferences. If no info,
   * default to 0.
   *
   * @return long returning current read status of each Preinstall location.
   * Default value in binary is `00.....00000000` indicating none of the locations are yet read.
   */
  public synchronized long getPreinstallPayloadReadStatus() {
    return getLong(PREFS_KEY_PREINSTALL_PAYLOAD_READ_STATUS, 0);
  }

  /**
   * Remove all key-value pairs from shared preferences.
   */
  public synchronized void clear() {
    this.sharedPreferences.edit().clear().apply();
  }

  /**
   * Write a string value to shared preferences.
   *
   * @param key   Key to be written to shared preferences
   * @param value Value to be written to shared preferences
   */
  private synchronized void saveString(final String key, final String value) {
    this.sharedPreferences.edit().putString(key, value).apply();
  }

  /**
   * Write a boolean value to shared preferences.
   *
   * @param key   Key to be written to shared preferences
   * @param value Value to be written to shared preferences
   */
  private synchronized void saveBoolean(final String key, final boolean value) {
    this.sharedPreferences.edit().putBoolean(key, value).apply();
  }

  /**
   * Write a long value to shared preferences.
   *
   * @param key   Key to be written to shared preferences
   * @param value Value to be written to shared preferences
   */
  private synchronized void saveLong(final String key, final long value) {
    this.sharedPreferences.edit().putLong(key, value).apply();
  }

  /**
   * Write a integer value to shared preferences.
   *
   * @param key   Key to be written to shared preferences
   * @param value Value to be written to shared preferences
   */
  private synchronized void saveInteger(final String key, final int value) {
    this.sharedPreferences.edit().putInt(key, value).apply();
  }

  /**
   * Get a string value from shared preferences.
   *
   * @param key Key for which string value should be retrieved
   * @return String value for given key saved in shared preferences (null if not found)
   */
  private synchronized String getString(final String key) {
    try {
      return this.sharedPreferences.getString(key, null);
    } catch (ClassCastException e) {
      return null;
    } catch (Throwable t) {
      if (key.equals(PREFS_KEY_RAW_REFERRERS)) {
        remove(PREFS_KEY_RAW_REFERRERS);
      }
      return null;
    }
  }

  /**
   * Get a boolean value from shared preferences.
   *
   * @param key          Key for which boolean value should be retrieved
   * @param defaultValue Default value to be returned if nothing found in shared preferences
   * @return Boolean value for given key saved in shared preferences
   */
  private synchronized boolean getBoolean(final String key, final boolean defaultValue) {
    try {
      return this.sharedPreferences.getBoolean(key, defaultValue);
    } catch (ClassCastException e) {
      return defaultValue;
    }
  }

  /**
   * Get a long value from shared preferences.
   *
   * @param key          Key for which long value should be retrieved
   * @param defaultValue Default value to be returned if nothing found in shared preferences
   * @return Long value for given key saved in shared preferences
   */
  private synchronized long getLong(final String key, final long defaultValue) {
    try {
      return this.sharedPreferences.getLong(key, defaultValue);
    } catch (ClassCastException e) {
      return defaultValue;
    }
  }

  /**
   * Remove a value saved with given key from shared preferences.
   *
   * @param key Key to be removed
   */
  private synchronized void remove(final String key) {
    this.sharedPreferences.edit().remove(key).apply();
  }
}
