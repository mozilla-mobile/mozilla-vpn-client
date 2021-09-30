package com.adjust.sdk;

import org.json.JSONObject;

/**
 * Adjust SDK
 * Created by Pedro Silva (@nonelse) on 16th February 2016.
 * Copyright \u00a9 2016-2018 Adjust GmbH. All rights reserved.
 */
public class AdjustSessionSuccess {
  public String adid;
  public String message;
  public String timestamp;
  public JSONObject jsonResponse;

  @Override
  public String toString() {
    return Util.formatString(
        "Session Success msg:%s time:%s adid:%s json:%s", message, timestamp, adid, jsonResponse);
  }
}
