package com.adjust.sdk;

import android.content.Context;

import com.adjust.sdk.network.IActivityPackageSender;
import com.adjust.sdk.network.UtilNetworking;

import java.net.URL;

import java.net.HttpURLConnection;

public class AdjustFactory {
  private static IPackageHandler packageHandler = null;
  private static IAttributionHandler attributionHandler = null;
  private static IActivityHandler activityHandler = null;
  private static ILogger logger = null;
  private static ISdkClickHandler sdkClickHandler = null;

  private static long timerInterval = -1;
  private static long timerStart = -1;
  private static long sessionInterval = -1;
  private static long subsessionInterval = -1;
  private static BackoffStrategy sdkClickBackoffStrategy = null;
  private static BackoffStrategy packageHandlerBackoffStrategy = null;
  private static BackoffStrategy installSessionBackoffStrategy = null;
  private static long maxDelayStart = -1;
  private static String baseUrl = null;
  private static String gdprUrl = null;
  private static String subscriptionUrl = null;
  private static UtilNetworking.IConnectionOptions connectionOptions = null;
  private static UtilNetworking.IHttpURLConnectionProvider httpURLConnectionProvider = null;
  private static boolean tryInstallReferrer = true;

  public static class URLGetConnection {
    HttpURLConnection httpURLConnection;
    URL url;
    URLGetConnection(HttpURLConnection httpURLConnection, URL url) {
      this.httpURLConnection = httpURLConnection;
      this.url = url;
    }
  }

  public static IPackageHandler getPackageHandler(IActivityHandler activityHandler, Context context,
      boolean startsSending, IActivityPackageSender packageHandlerActivityPackageSender) {
    if (packageHandler == null) {
      return new PackageHandler(
          activityHandler, context, startsSending, packageHandlerActivityPackageSender);
    }
    packageHandler.init(
        activityHandler, context, startsSending, packageHandlerActivityPackageSender);
    return packageHandler;
  }

  public static ILogger getLogger() {
    if (logger == null) {
      // Logger needs to be "static" to retain the configuration throughout the app
      logger = new Logger();
    }
    return logger;
  }

  public static long getTimerInterval() {
    if (timerInterval == -1) {
      return Constants.ONE_MINUTE;
    }
    return timerInterval;
  }

  public static long getTimerStart() {
    if (timerStart == -1) {
      return Constants.ONE_MINUTE;
    }
    return timerStart;
  }

  public static long getSessionInterval() {
    if (sessionInterval == -1) {
      return Constants.THIRTY_MINUTES;
    }
    return sessionInterval;
  }

  public static long getSubsessionInterval() {
    if (subsessionInterval == -1) {
      return Constants.ONE_SECOND;
    }
    return subsessionInterval;
  }

  public static BackoffStrategy getSdkClickBackoffStrategy() {
    if (sdkClickBackoffStrategy == null) {
      return BackoffStrategy.SHORT_WAIT;
    }
    return sdkClickBackoffStrategy;
  }

  public static BackoffStrategy getPackageHandlerBackoffStrategy() {
    if (packageHandlerBackoffStrategy == null) {
      return BackoffStrategy.LONG_WAIT;
    }
    return packageHandlerBackoffStrategy;
  }

  public static BackoffStrategy getInstallSessionBackoffStrategy() {
    if (installSessionBackoffStrategy == null) {
      return BackoffStrategy.SHORT_WAIT;
    }
    return installSessionBackoffStrategy;
  }

  public static IActivityHandler getActivityHandler(AdjustConfig config) {
    if (activityHandler == null) {
      return ActivityHandler.getInstance(config);
    }
    activityHandler.init(config);
    return activityHandler;
  }

  public static IAttributionHandler getAttributionHandler(IActivityHandler activityHandler,
      boolean startsSending, IActivityPackageSender packageHandlerActivityPackageSender) {
    if (attributionHandler == null) {
      return new AttributionHandler(
          activityHandler, startsSending, packageHandlerActivityPackageSender);
    }
    attributionHandler.init(activityHandler, startsSending, packageHandlerActivityPackageSender);
    return attributionHandler;
  }

  public static ISdkClickHandler getSdkClickHandler(IActivityHandler activityHandler,
      boolean startsSending, IActivityPackageSender packageHandlerActivityPackageSender) {
    if (sdkClickHandler == null) {
      return new SdkClickHandler(
          activityHandler, startsSending, packageHandlerActivityPackageSender);
    }

    sdkClickHandler.init(activityHandler, startsSending, packageHandlerActivityPackageSender);
    return sdkClickHandler;
  }

  public static long getMaxDelayStart() {
    if (maxDelayStart == -1) {
      return Constants.ONE_SECOND * 10; // 10 seconds
    }
    return maxDelayStart;
  }

  public static String getBaseUrl() {
    return AdjustFactory.baseUrl;
  }

  public static String getGdprUrl() {
    return AdjustFactory.gdprUrl;
  }

  public static String getSubscriptionUrl() {
    return AdjustFactory.subscriptionUrl;
  }

  public static UtilNetworking.IConnectionOptions getConnectionOptions() {
    if (connectionOptions == null) {
      return UtilNetworking.createDefaultConnectionOptions();
    }
    return connectionOptions;
  }

  public static UtilNetworking.IHttpURLConnectionProvider getHttpURLConnectionProvider() {
    if (httpURLConnectionProvider == null) {
      return UtilNetworking.createDefaultHttpURLConnectionProvider();
    }
    return httpURLConnectionProvider;
  }

  public static boolean getTryInstallReferrer() {
    return tryInstallReferrer;
  }

  public static void setPackageHandler(IPackageHandler packageHandler) {
    AdjustFactory.packageHandler = packageHandler;
  }

  public static void setLogger(ILogger logger) {
    AdjustFactory.logger = logger;
  }

  public static void setTimerInterval(long timerInterval) {
    AdjustFactory.timerInterval = timerInterval;
  }

  public static void setTimerStart(long timerStart) {
    AdjustFactory.timerStart = timerStart;
  }

  public static void setSessionInterval(long sessionInterval) {
    AdjustFactory.sessionInterval = sessionInterval;
  }

  public static void setSubsessionInterval(long subsessionInterval) {
    AdjustFactory.subsessionInterval = subsessionInterval;
  }

  public static void setSdkClickBackoffStrategy(BackoffStrategy sdkClickBackoffStrategy) {
    AdjustFactory.sdkClickBackoffStrategy = sdkClickBackoffStrategy;
  }

  public static void setPackageHandlerBackoffStrategy(
      BackoffStrategy packageHandlerBackoffStrategy) {
    AdjustFactory.packageHandlerBackoffStrategy = packageHandlerBackoffStrategy;
  }

  public static void setActivityHandler(IActivityHandler activityHandler) {
    AdjustFactory.activityHandler = activityHandler;
  }

  public static void setAttributionHandler(IAttributionHandler attributionHandler) {
    AdjustFactory.attributionHandler = attributionHandler;
  }

  public static void setSdkClickHandler(ISdkClickHandler sdkClickHandler) {
    AdjustFactory.sdkClickHandler = sdkClickHandler;
  }

  public static void setBaseUrl(String baseUrl) {
    AdjustFactory.baseUrl = baseUrl;
  }

  public static void setGdprUrl(String gdprUrl) {
    AdjustFactory.gdprUrl = gdprUrl;
  }

  public static void setSubscriptionUrl(String subscriptionUrl) {
    AdjustFactory.subscriptionUrl = subscriptionUrl;
  }

  public static void setConnectionOptions(UtilNetworking.IConnectionOptions connectionOptions) {
    AdjustFactory.connectionOptions = connectionOptions;
  }

  public static void setHttpURLConnectionProvider(
      UtilNetworking.IHttpURLConnectionProvider httpURLConnectionProvider) {
    AdjustFactory.httpURLConnectionProvider = httpURLConnectionProvider;
  }

  public static void setTryInstallReferrer(boolean tryInstallReferrer) {
    AdjustFactory.tryInstallReferrer = tryInstallReferrer;
  }

  public static void enableSigning() {
    AdjustSigner.enableSigning(getLogger());
  }

  public static void disableSigning() {
    AdjustSigner.disableSigning(getLogger());
  }

  private static String byte2HexFormatted(byte[] arr) {
    StringBuilder str = new StringBuilder(arr.length * 2);

    for (int i = 0; i < arr.length; i++) {
      String h = Integer.toHexString(arr[i]);
      int l = h.length();

      if (l == 1) {
        h = "0" + h;
      }

      if (l > 2) {
        h = h.substring(l - 2, l);
      }

      str.append(h.toUpperCase());

      // if (i < (arr.length - 1)) str.append(':');
    }
    return str.toString();
  }

  public static void teardown(Context context) {
    if (context != null) {
      ActivityHandler.deleteState(context);
      PackageHandler.deleteState(context);
    }
    packageHandler = null;
    attributionHandler = null;
    activityHandler = null;
    logger = null;
    sdkClickHandler = null;

    timerInterval = -1;
    timerStart = -1;
    sessionInterval = -1;
    subsessionInterval = -1;
    sdkClickBackoffStrategy = null;
    packageHandlerBackoffStrategy = null;
    maxDelayStart = -1;
    baseUrl = Constants.BASE_URL;
    gdprUrl = Constants.GDPR_URL;
    subscriptionUrl = Constants.SUBSCRIPTION_URL;
    connectionOptions = null;
    httpURLConnectionProvider = null;
    tryInstallReferrer = true;
  }
}
