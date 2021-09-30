package com.adjust.sdk;

import android.content.Context;

/**
 * Created by nonelse on 08.01.2018
 */

public class AdjustTestOptions {
  public Context context;
  public String baseUrl;
  public String gdprUrl;
  public String subscriptionUrl;
  public String basePath;
  public String gdprPath;
  public String subscriptionPath;
  public Long timerIntervalInMilliseconds;
  public Long timerStartInMilliseconds;
  public Long sessionIntervalInMilliseconds;
  public Long subsessionIntervalInMilliseconds;
  public Boolean teardown;
  public Boolean tryInstallReferrer = false;
  public Boolean noBackoffWait;
  public Boolean enableSigning;
  public Boolean disableSigning;
}
