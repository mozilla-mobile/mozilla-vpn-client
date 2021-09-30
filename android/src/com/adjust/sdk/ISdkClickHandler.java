package com.adjust.sdk;

import com.adjust.sdk.network.IActivityPackageSender;

/**
 * SdkClickHandler interface.
 *
 * @author Pedro Silva (nonelse)
 * @since 31st March 2016
 */

public interface ISdkClickHandler {
  /**
   * Initialise SdkClickHandler instance.
   *
   * @param activityHandler Activity handler instance.
   * @param startsSending   Is sending paused?
   */
  void init(IActivityHandler activityHandler, boolean startsSending,
      IActivityPackageSender sdkClickHandlerActivityPackageSender);

  /**
   * Pause sending from SdkClickHandler.
   */
  void pauseSending();

  /**
   * Resume sending from SdkClickHandler.
   */
  void resumeSending();

  /**
   * Send sdk_click package.
   *
   * @param sdkClick sdk_click package to be sent.
   */
  void sendSdkClick(ActivityPackage sdkClick);

  /**
   * Send sdk_click packages made from all the persisted intent type referrers.
   */
  void sendReftagReferrers();

  /**
   * Send sdk_click package carrying preinstall info.
   */
  void sendPreinstallPayload(String payload, String location);

  /**
   * Teardown SdkClickHandler instance.
   */
  void teardown();
}
