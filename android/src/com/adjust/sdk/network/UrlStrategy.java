package com.adjust.sdk.network;

import com.adjust.sdk.ActivityKind;
import com.adjust.sdk.Constants;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static com.adjust.sdk.AdjustConfig.DATA_RESIDENCY_TR;
import static com.adjust.sdk.AdjustConfig.DATA_RESIDENCY_US;
import static com.adjust.sdk.AdjustConfig.URL_STRATEGY_CHINA;
import static com.adjust.sdk.AdjustConfig.URL_STRATEGY_INDIA;
import static com.adjust.sdk.AdjustConfig.DATA_RESIDENCY_EU;

public class UrlStrategy {
  private static final String BASE_URL_INDIA = "http://app.adjust.net.in";
  private static final String GDPR_URL_INDIA = "http://gdpr.adjust.net.in";
  private static final String SUBSCRIPTION_URL_INDIA = "http://subscription.adjust.net.in";

  private static final String BASE_URL_CHINA = "http://app.adjust.world";
  private static final String GDPR_URL_CHINA = "http://gdpr.adjust.world";
  private static final String SUBSCRIPTION_URL_CHINA = "http://subscription.adjust.world";

  private static final String BASE_URL_EU = "http://app.eu.adjust.com";
  private static final String GDPR_URL_EU = "http://gdpr.eu.adjust.com";
  private static final String SUBSCRIPTION_URL_EU = "http://subscription.eu.adjust.com";

  private static final String BASE_URL_TR = "http://app.tr.adjust.com";
  private static final String GDPR_URL_TR = "http://gdpr.tr.adjust.com";
  private static final String SUBSCRIPTION_URL_TR = "http://subscription.tr.adjust.com";

  private static final String BASE_URL_US = "http://app.us.adjust.com";
  private static final String GDPR_URL_US = "http://gdpr.us.adjust.com";
  private static final String SUBSCRIPTION_URL_US = "http://subscription.us.adjust.com";

  private final String baseUrlOverwrite;
  private final String gdprUrlOverwrite;
  private final String subscriptionUrlOverwrite;

  final List<String> baseUrlChoicesList;
  final List<String> gdprUrlChoicesList;
  final List<String> subscriptionUrlChoicesList;
  boolean wasLastAttemptSuccess;
  int choiceIndex;
  int startingChoiceIndex;
  boolean wasLastAttemptWithOverwrittenUrl;

  public UrlStrategy(final String baseUrlOverwrite, final String gdprUrlOverwrite,
      final String subscriptionUrlOverwrite, final String adjustUrlStrategy) {
    this.baseUrlOverwrite = baseUrlOverwrite;
    this.gdprUrlOverwrite = gdprUrlOverwrite;
    this.subscriptionUrlOverwrite = subscriptionUrlOverwrite;

    baseUrlChoicesList = baseUrlChoices(adjustUrlStrategy);
    gdprUrlChoicesList = gdprUrlChoices(adjustUrlStrategy);
    subscriptionUrlChoicesList = subscriptionUrlChoices(adjustUrlStrategy);

    wasLastAttemptSuccess = false;
    choiceIndex = 0;
    startingChoiceIndex = 0;
    wasLastAttemptWithOverwrittenUrl = false;
  }

  public void resetAfterSuccess() {
    startingChoiceIndex = choiceIndex;
    wasLastAttemptSuccess = true;
  }

  @SuppressWarnings("UnnecessaryLocalVariable")
  public boolean shouldRetryAfterFailure(final ActivityKind activityKind) {
    wasLastAttemptSuccess = false;

    // does not need to "rotate" choice index
    //  since it will use the same overwritten url
    //  might as well stop retrying in the same sending "session"
    //  and let the backoff strategy pick it up
    if (wasLastAttemptWithOverwrittenUrl) {
      return false;
    }

    int choiceListSize;

    if (activityKind == ActivityKind.GDPR) {
      choiceListSize = gdprUrlChoicesList.size();
    } else if (activityKind == ActivityKind.SUBSCRIPTION) {
      choiceListSize = subscriptionUrlChoicesList.size();
    } else {
      choiceListSize = baseUrlChoicesList.size();
    }

    final int nextChoiceIndex = (choiceIndex + 1) % choiceListSize;
    choiceIndex = nextChoiceIndex;

    final boolean nextChoiceHasNotReturnedToStartingChoice = choiceIndex != startingChoiceIndex;

    return nextChoiceHasNotReturnedToStartingChoice;
  }

  public String targetUrlByActivityKind(final ActivityKind activityKind) {
    if (activityKind == ActivityKind.GDPR) {
      if (gdprUrlOverwrite != null) {
        wasLastAttemptWithOverwrittenUrl = true;
        return gdprUrlOverwrite;
      } else {
        wasLastAttemptWithOverwrittenUrl = false;
        return gdprUrlChoicesList.get(choiceIndex);
      }
    } else if (activityKind == ActivityKind.SUBSCRIPTION) {
      if (subscriptionUrlOverwrite != null) {
        wasLastAttemptWithOverwrittenUrl = true;
        return subscriptionUrlOverwrite;
      } else {
        wasLastAttemptWithOverwrittenUrl = false;
        return subscriptionUrlChoicesList.get(choiceIndex);
      }
    } else {
      if (baseUrlOverwrite != null) {
        wasLastAttemptWithOverwrittenUrl = true;
        return baseUrlOverwrite;
      } else {
        wasLastAttemptWithOverwrittenUrl = false;
        return baseUrlChoicesList.get(choiceIndex);
      }
    }
  }

  private static List<String> baseUrlChoices(final String urlStrategy) {
    if (URL_STRATEGY_INDIA.equals(urlStrategy)) {
      return Arrays.asList(BASE_URL_INDIA, Constants.BASE_URL);
    } else if (URL_STRATEGY_CHINA.equals(urlStrategy)) {
      return Arrays.asList(BASE_URL_CHINA, Constants.BASE_URL);
    } else if (DATA_RESIDENCY_EU.equals(urlStrategy)) {
      return Collections.singletonList(BASE_URL_EU);
    } else if (DATA_RESIDENCY_TR.equals(urlStrategy)) {
      return Collections.singletonList(BASE_URL_TR);
    } else if (DATA_RESIDENCY_US.equals(urlStrategy)) {
      return Collections.singletonList(BASE_URL_US);
    } else {
      return Arrays.asList(Constants.BASE_URL, BASE_URL_INDIA, BASE_URL_CHINA);
    }
  }
  private static List<String> gdprUrlChoices(final String urlStrategy) {
    if (URL_STRATEGY_INDIA.equals(urlStrategy)) {
      return Arrays.asList(GDPR_URL_INDIA, Constants.GDPR_URL);
    } else if (URL_STRATEGY_CHINA.equals(urlStrategy)) {
      return Arrays.asList(GDPR_URL_CHINA, Constants.GDPR_URL);
    } else if (DATA_RESIDENCY_EU.equals(urlStrategy)) {
      return Collections.singletonList(GDPR_URL_EU);
    } else if (DATA_RESIDENCY_TR.equals(urlStrategy)) {
      return Collections.singletonList(GDPR_URL_TR);
    } else if (DATA_RESIDENCY_US.equals(urlStrategy)) {
      return Collections.singletonList(GDPR_URL_US);
    } else {
      return Arrays.asList(Constants.GDPR_URL, GDPR_URL_INDIA, GDPR_URL_CHINA);
    }
  }
  private static List<String> subscriptionUrlChoices(final String urlStrategy) {
    if (URL_STRATEGY_INDIA.equals(urlStrategy)) {
      return Arrays.asList(SUBSCRIPTION_URL_INDIA, Constants.SUBSCRIPTION_URL);
    } else if (URL_STRATEGY_CHINA.equals(urlStrategy)) {
      return Arrays.asList(SUBSCRIPTION_URL_CHINA, Constants.SUBSCRIPTION_URL);
    } else if (DATA_RESIDENCY_EU.equals(urlStrategy)) {
      return Collections.singletonList(SUBSCRIPTION_URL_EU);
    } else if (DATA_RESIDENCY_TR.equals(urlStrategy)) {
      return Collections.singletonList(SUBSCRIPTION_URL_TR);
    } else if (DATA_RESIDENCY_US.equals(urlStrategy)) {
      return Collections.singletonList(SUBSCRIPTION_URL_US);
    } else {
      return Arrays.asList(
          Constants.SUBSCRIPTION_URL, SUBSCRIPTION_URL_INDIA, SUBSCRIPTION_URL_CHINA);
    }
  }
}
