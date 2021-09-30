package com.adjust.sdk;

import org.json.JSONObject;

/**
 * Adjust SDK
 * Created by Pedro Silva (@nonelse) on 9th February 2016.
 * Copyright \u00a9 2016-2018 Adjust GmbH. All rights reserved.
 */
public class SessionResponseData extends ResponseData {
  private String sdkPlatform;

  public SessionResponseData(final ActivityPackage activityPackage) {
    this.sdkPlatform = Util.getSdkPrefixPlatform(activityPackage.getClientSdk());
  }

  public AdjustSessionSuccess getSuccessResponseData() {
    if (!success) {
      return null;
    }

    AdjustSessionSuccess successResponseData = new AdjustSessionSuccess();
    if ("unity".equals(this.sdkPlatform)) {
      // Unity platform.
      successResponseData.message = message != null ? message : "";
      successResponseData.timestamp = timestamp != null ? timestamp : "";
      successResponseData.adid = adid != null ? adid : "";
      successResponseData.jsonResponse = jsonResponse != null ? jsonResponse : new JSONObject();
    } else {
      // Rest of all platforms.
      successResponseData.message = message;
      successResponseData.timestamp = timestamp;
      successResponseData.adid = adid;
      successResponseData.jsonResponse = jsonResponse;
    }

    return successResponseData;
  }

  public AdjustSessionFailure getFailureResponseData() {
    if (success) {
      return null;
    }

    AdjustSessionFailure failureResponseData = new AdjustSessionFailure();
    if ("unity".equals(this.sdkPlatform)) {
      // Unity platform.
      failureResponseData.message = message != null ? message : "";
      failureResponseData.timestamp = timestamp != null ? timestamp : "";
      failureResponseData.adid = adid != null ? adid : "";
      failureResponseData.willRetry = willRetry;
      failureResponseData.jsonResponse = jsonResponse != null ? jsonResponse : new JSONObject();
    } else {
      // Rest of all platforms.
      failureResponseData.message = message;
      failureResponseData.timestamp = timestamp;
      failureResponseData.adid = adid;
      failureResponseData.willRetry = willRetry;
      failureResponseData.jsonResponse = jsonResponse;
    }

    return failureResponseData;
  }
}
