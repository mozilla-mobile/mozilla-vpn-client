/*
Criteria for addon:
 - Timing: 30 days, 90 days and 180 days after “Subscription start”
 - Plan purchase source: not IAP
 - Plan renewal cycle: On annual plan
 - Plan type: Not already on a bundle
 - Geo: US-only - Confirming via location class AND payment in USD
 - Client version: Client is at least v2.29 (as some of the APIs used here were
added in 2.29) - handled in manifest.json
 */

(function(api, condition) {
// Run when TaskGetSubscriptionDetails returns something different than what is
// in Settings (often from signing in/out but can be any change in the
// subscription)
api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());

// Run on app launch for signed-in users when there subscription data is loaded
// into memory from settings
api.connectSignal(
    api.subscriptionData, 'initialized', () => computeCondition());

// Run on app launch after getting current location
api.connectSignal(api.location, 'changed', () => computeCondition());

// This is for already-running clients that receive this addon while the client
// is launched and signed in. (These users may not see a change in
// subscriptionData for a long time.)
computeCondition()

function isAnnualPlan() {
  return api.subscriptionData.planBillingInterval ===
      api.subscriptionData.BillingIntervalYearly
}

function isWebSubscription() {
  return api.subscriptionData.type === api.subscriptionData.SubscriptionWeb
}

function isBundleSubscription() {
  return api.subscriptionData.isBundleSubscription
}

function isLocatedInUs() {
  return api.location.countryCode.toUpperCase() === 'US'
}

function isUsd() {
  return api.subscriptionData.planCurrency === 'USD'
}

function computeCondition() {
  // If subscriptionData is not initialized, then return
  if (api.subscriptionData.createdAt <= 0) {
    condition.disable()
    return
  }

  if (!isAnnualPlan() || !isWebSubscription() || isBundleSubscription() ||
      !isLocatedInUs() || !isUsd()) {
    condition.disable()
    return
  }

  let baseDevTime = 1000  // 1 second = 1000 ms
  let baseProdTime = 1000 * 60 * 60 *
      24  // 1 day = 1000 ms/sec * 60 sec/min * 60 min/hr * 24 hr/day
  let baseTime = api.env.inProduction ? baseProdTime : baseDevTime

  // First message: Visible 30-87
  // Second message: Visible 90-177
  // Third message; Visible 180+

  let now = Date.now()
  let startTime = api.subscriptionData.createdAt + 180 * baseTime

  if (now < startTime) {
    // Before start, don't show message
    api.setTimedCallback(startTime - now, () => computeCondition());
    condition.disable()
  }
  else {
    // Between start and end time, show message
    const ADDON_RELEASE_DATE = 1750696005
    api.addon.date = Math.max(startTime / 1000, ADDON_RELEASE_DATE);
    condition.enable()
  }
}
});
