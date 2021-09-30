package com.adjust.sdk;

import org.json.JSONObject;

/**
 * Adjust SDK
 * Created by Pedro Silva (@nonelse) on 4th January 2016.
 * Copyright \u00a9 2016-2018 Adjust GmbH. All rights reserved.
 */
public class AdjustEventSuccess {
  public String adid;
  public String message;
  public String timestamp;
  public String eventToken;
  public String callbackId;
  public JSONObject jsonResponse;

  @Override
  public String toString() {
    return Util.formatString("Event Success msg:%s time:%s adid:%s event:%s cid:%s json:%s",
        message, timestamp, adid, eventToken, callbackId, jsonResponse);
  }
}
