//
//  IAttributionHandler.java
//  Adjust SDK
//
//  Created by Pedro Silva (@nonelse) on 15th December 2014.
//  Copyright (c) 2014-2018 Adjust GmbH. All rights reserved.
//

package com.adjust.sdk;

import com.adjust.sdk.network.IActivityPackageSender;

public interface IAttributionHandler {
  void init(IActivityHandler activityHandler, boolean startsSending,
      IActivityPackageSender attributionHandlerActivityPackageSender);
  void checkSessionResponse(SessionResponseData sessionResponseData);
  void checkSdkClickResponse(SdkClickResponseData sdkClickResponseData);
  void pauseSending();
  void resumeSending();
  void getAttribution();
  void teardown();
}
