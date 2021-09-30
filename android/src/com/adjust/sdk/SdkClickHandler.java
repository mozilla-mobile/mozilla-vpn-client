package com.adjust.sdk;

import com.adjust.sdk.network.IActivityPackageSender;
import com.adjust.sdk.scheduler.SingleThreadCachedScheduler;
import com.adjust.sdk.scheduler.ThreadScheduler;

import org.json.JSONArray;
import org.json.JSONException;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * SdkClickHandler class.
 *
 * @author Pedro Silva (nonelse)
 * @since 31st March 2016
 */

public class SdkClickHandler implements ISdkClickHandler {
  /**
   * Divisor for milliseconds -> seconds conversion.
   */
  private static final double MILLISECONDS_TO_SECONDS_DIVISOR = 1000.0;

  /**
   * SdkClickHandler scheduled executor source.
   */
  private static final String SCHEDULED_EXECUTOR_SOURCE = "SdkClickHandler";

  /**
   * Intent based referrer source name inside of sdk_click package.
   */
  private static final String SOURCE_REFTAG = "reftag";

  /**
   * Install referrer service referrer source name inside of sdk_click package.
   */
  private static final String SOURCE_INSTALL_REFERRER = "install_referrer";

  /**
   * Indicates whether SdkClickHandler is paused or not.
   */
  private boolean paused;

  /**
   * Adjust logger.
   */
  private ILogger logger;

  /**
   * Backoff strategy.
   */
  private BackoffStrategy backoffStrategy;

  /**
   * Sending queue.
   */
  private List<ActivityPackage> packageQueue;

  /**
   * Custom actions scheduled executor.
   */
  private ThreadScheduler scheduler;

  /**
   * ActivityHandler instance.
   */
  private WeakReference<IActivityHandler> activityHandlerWeakRef;

  private IActivityPackageSender activityPackageSender;

  /**
   * SdkClickHandler constructor.
   *
   * @param activityHandler ActivityHandler reference
   * @param startsSending   Is sending paused?
   */
  public SdkClickHandler(final IActivityHandler activityHandler, final boolean startsSending,
      final IActivityPackageSender sdkClickHandlerActivityPackageSender) {
    init(activityHandler, startsSending, sdkClickHandlerActivityPackageSender);
    this.logger = AdjustFactory.getLogger();
    backoffStrategy = AdjustFactory.getSdkClickBackoffStrategy();
    scheduler = new SingleThreadCachedScheduler("SdkClickHandler");
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void init(final IActivityHandler activityHandler, final boolean startsSending,
      final IActivityPackageSender sdkClickHandlerActivityPackageSender) {
    paused = !startsSending;
    packageQueue = new ArrayList<ActivityPackage>();
    activityHandlerWeakRef = new WeakReference<IActivityHandler>(activityHandler);
    activityPackageSender = sdkClickHandlerActivityPackageSender;
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void pauseSending() {
    paused = true;
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void resumeSending() {
    paused = false;

    sendNextSdkClick();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void sendSdkClick(final ActivityPackage sdkClick) {
    scheduler.submit(new Runnable() {
      @Override
      public void run() {
        packageQueue.add(sdkClick);

        logger.debug("Added sdk_click %d", packageQueue.size());
        logger.verbose("%s", sdkClick.getExtendedString());

        sendNextSdkClick();
      }
    });
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void sendReftagReferrers() {
    scheduler.submit(new Runnable() {
      @Override
      public void run() {
        IActivityHandler activityHandler = activityHandlerWeakRef.get();
        SharedPreferencesManager sharedPreferencesManager =
            new SharedPreferencesManager(activityHandler.getContext());
        try {
          JSONArray rawReferrerArray = sharedPreferencesManager.getRawReferrerArray();
          boolean hasRawReferrersBeenChanged = false;

          for (int i = 0; i < rawReferrerArray.length(); i++) {
            JSONArray savedRawReferrer = rawReferrerArray.getJSONArray(i);

            int savedRawReferrerState = savedRawReferrer.optInt(2, -1);

            // Don't send the one already sending or sent.
            if (savedRawReferrerState != 0) {
              continue;
            }

            String savedRawReferrerString = savedRawReferrer.optString(0, null);
            long savedClickTime = savedRawReferrer.optLong(1, -1);
            // Mark install referrer as being sent.
            savedRawReferrer.put(2, 1);
            hasRawReferrersBeenChanged = true;

            // Create sdk click
            ActivityPackage sdkClickPackage =
                PackageFactory.buildReftagSdkClickPackage(savedRawReferrerString, savedClickTime,
                    activityHandler.getActivityState(), activityHandler.getAdjustConfig(),
                    activityHandler.getDeviceInfo(), activityHandler.getSessionParameters());

            // Send referrer sdk_click package.
            sendSdkClick(sdkClickPackage);
          }

          if (hasRawReferrersBeenChanged) {
            sharedPreferencesManager.saveRawReferrerArray(rawReferrerArray);
          }
        } catch (JSONException e) {
          logger.error("Send saved raw referrers error (%s)", e.getMessage());
        }
      }
    });
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void sendPreinstallPayload(
      final String preinstallPayload, final String preinstallLocation) {
    scheduler.submit(new Runnable() {
      @Override
      public void run() {
        IActivityHandler activityHandler = activityHandlerWeakRef.get();
        if (activityHandler == null) {
          return;
        }

        // Create sdk click
        ActivityPackage sdkClickPackage =
            PackageFactory.buildPreinstallSdkClickPackage(preinstallPayload, preinstallLocation,
                activityHandler.getActivityState(), activityHandler.getAdjustConfig(),
                activityHandler.getDeviceInfo(), activityHandler.getSessionParameters());

        // Send preinstall info sdk_click package.
        sendSdkClick(sdkClickPackage);
      }
    });
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void teardown() {
    logger.verbose("SdkClickHandler teardown");

    if (scheduler != null) {
      scheduler.teardown();
    }

    if (packageQueue != null) {
      packageQueue.clear();
    }

    if (activityHandlerWeakRef != null) {
      activityHandlerWeakRef.clear();
    }

    logger = null;
    packageQueue = null;
    backoffStrategy = null;
    scheduler = null;
  }

  /**
   * Send next sdk_click package from the queue.
   */
  private void sendNextSdkClick() {
    scheduler.submit(new Runnable() {
      @Override
      public void run() {
        sendNextSdkClickI();
      }
    });
  }

  /**
   * Send next sdk_click package from the queue (runs within scheduled executor).
   */
  private void sendNextSdkClickI() {
    IActivityHandler activityHandler = activityHandlerWeakRef.get();
    if (activityHandler.getActivityState() == null) {
      return;
    }
    if (activityHandler.getActivityState().isGdprForgotten) {
      return;
    }
    if (paused) {
      return;
    }
    if (packageQueue.isEmpty()) {
      return;
    }

    final ActivityPackage sdkClickPackage = packageQueue.remove(0);
    int retries = sdkClickPackage.getRetries();

    Runnable runnable = new Runnable() {
      @Override
      public void run() {
        sendSdkClickI(sdkClickPackage);
        sendNextSdkClick();
      }
    };

    if (retries <= 0) {
      runnable.run();
      return;
    }

    long waitTimeMilliSeconds = Util.getWaitingTime(retries, backoffStrategy);
    double waitTimeSeconds = waitTimeMilliSeconds / MILLISECONDS_TO_SECONDS_DIVISOR;
    String secondsString = Util.SecondsDisplayFormat.format(waitTimeSeconds);

    logger.verbose(
        "Waiting for %s seconds before retrying sdk_click for the %d time", secondsString, retries);

    scheduler.schedule(runnable, waitTimeMilliSeconds);
  }

  /**
   * Send sdk_click package passed as the parameter (runs within scheduled executor).
   *
   * @param sdkClickPackage sdk_click package to be sent.
   */
  private void sendSdkClickI(final ActivityPackage sdkClickPackage) {
    IActivityHandler activityHandler = activityHandlerWeakRef.get();
    String source = sdkClickPackage.getParameters().get("source");
    boolean isReftag = source != null && source.equals(SOURCE_REFTAG);
    String rawReferrerString = sdkClickPackage.getParameters().get("raw_referrer");

    if (isReftag) {
      // Check before sending if referrer was removed already.
      SharedPreferencesManager sharedPreferencesManager =
          new SharedPreferencesManager(activityHandler.getContext());

      JSONArray rawReferrer = sharedPreferencesManager.getRawReferrer(
          rawReferrerString, sdkClickPackage.getClickTimeInMilliseconds());

      if (rawReferrer == null) {
        return;
      }
    }

    boolean isInstallReferrer = source != null && source.equals(SOURCE_INSTALL_REFERRER);
    long clickTime = -1;
    long installBegin = -1;
    String installReferrer = null;
    long clickTimeServer = -1;
    long installBeginServer = -1;
    String installVersion = null;
    Boolean googlePlayInstant = null;
    String referrerApi = null;

    if (isInstallReferrer) {
      // Check if install referrer information is saved to activity state.
      // If yes, we have successfully sent it at earlier point and no need to do it again.
      // If not, proceed with sending of sdk_click package for install referrer.
      clickTime = sdkClickPackage.getClickTimeInSeconds();
      installBegin = sdkClickPackage.getInstallBeginTimeInSeconds();
      installReferrer = sdkClickPackage.getParameters().get("referrer");
      clickTimeServer = sdkClickPackage.getClickTimeServerInSeconds();
      installBeginServer = sdkClickPackage.getInstallBeginTimeServerInSeconds();
      installVersion = sdkClickPackage.getInstallVersion();
      googlePlayInstant = sdkClickPackage.getGooglePlayInstant();
      referrerApi = sdkClickPackage.getParameters().get("referrer_api");
    }

    boolean isPreinstall = source != null && source.equals(Constants.PREINSTALL);

    Map<String, String> sendingParameters = generateSendingParametersI();

    ResponseData responseData =
        activityPackageSender.sendActivityPackageSync(sdkClickPackage, sendingParameters);

    if (!(responseData instanceof SdkClickResponseData)) {
      return;
    }

    SdkClickResponseData sdkClickResponseData = (SdkClickResponseData) responseData;

    if (sdkClickResponseData.willRetry) {
      retrySendingI(sdkClickPackage);
      return;
    }

    if (activityHandler == null) {
      return;
    }

    if (sdkClickResponseData.trackingState == TrackingState.OPTED_OUT) {
      activityHandler.gotOptOutResponse();
      return;
    }

    if (isReftag) {
      // Remove referrer from shared preferences after sdk_click is sent.
      SharedPreferencesManager sharedPreferencesManager =
          new SharedPreferencesManager(activityHandler.getContext());

      sharedPreferencesManager.removeRawReferrer(
          rawReferrerString, sdkClickPackage.getClickTimeInMilliseconds());
    }

    if (isInstallReferrer) {
      // After successfully sending install referrer, store sent values in activity state.
      sdkClickResponseData.clickTime = clickTime;
      sdkClickResponseData.installBegin = installBegin;
      sdkClickResponseData.installReferrer = installReferrer;
      sdkClickResponseData.clickTimeServer = clickTimeServer;
      sdkClickResponseData.installBeginServer = installBeginServer;
      sdkClickResponseData.installVersion = installVersion;
      sdkClickResponseData.googlePlayInstant = googlePlayInstant;
      sdkClickResponseData.referrerApi = referrerApi;
      sdkClickResponseData.isInstallReferrer = true;
    }

    if (isPreinstall) {
      String payloadLocation = sdkClickPackage.getParameters().get("found_location");
      if (payloadLocation != null && !payloadLocation.isEmpty()) {
        // update preinstall flag in shared preferences after sdk_click is sent.
        SharedPreferencesManager sharedPreferencesManager =
            new SharedPreferencesManager(activityHandler.getContext());

        if (Constants.SYSTEM_INSTALLER_REFERRER.equalsIgnoreCase(payloadLocation)) {
          sharedPreferencesManager.removePreinstallReferrer();
        } else {
          long currentStatus = sharedPreferencesManager.getPreinstallPayloadReadStatus();
          long updatedStatus = PreinstallUtil.markAsRead(payloadLocation, currentStatus);
          sharedPreferencesManager.setPreinstallPayloadReadStatus(updatedStatus);
        }
      }
    }

    activityHandler.finishedTrackingActivity(sdkClickResponseData);
  }
  private Map<String, String> generateSendingParametersI() {
    HashMap<String, String> sendingParameters = new HashMap<>();

    long now = System.currentTimeMillis();
    String dateString = Util.dateFormatter.format(now);

    PackageBuilder.addString(sendingParameters, "sent_at", dateString);

    int queueSize = packageQueue.size() - 1;
    if (queueSize > 0) {
      PackageBuilder.addLong(sendingParameters, "queue_size", queueSize);
    }
    return sendingParameters;
  }

  /**
   * Retry sending of the sdk_click package passed as the parameter (runs within scheduled
   * executor).
   *
   * @param sdkClickPackage sdk_click package to be retried.
   */
  private void retrySendingI(final ActivityPackage sdkClickPackage) {
    int retries = sdkClickPackage.increaseRetries();

    logger.error("Retrying sdk_click package for the %d time", retries);

    sendSdkClick(sdkClickPackage);
  }

  /**
   * Print error log messages (runs within scheduled executor).
   *
   * @param sdkClickPackage sdk_click package for which error occured.
   * @param message         Message content.
   * @param throwable       Throwable to read the reason of the error.
   */
  private void logErrorMessageI(
      final ActivityPackage sdkClickPackage, final String message, final Throwable throwable) {
    final String packageMessage = sdkClickPackage.getFailureMessage();
    final String reasonString = Util.getReasonString(message, throwable);
    final String finalMessage = Util.formatString("%s. (%s)", packageMessage, reasonString);

    logger.error(finalMessage);
  }
}
