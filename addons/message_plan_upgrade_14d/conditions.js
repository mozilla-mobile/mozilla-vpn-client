(function(api, condition) {
  function computeCondition() {

    //Do not show message to non-web (eg IAP) or non-monthly (eg yearly) subscribers
    if (api.subscriptionData.type !== api.subscriptionData.SubscriptionWeb &&
        api.subscriptionData.planBillingInterval !== api.subscriptionData.BillingIntervalMonthly) {
      condition.disable()
      return
    }

    //Show this message after 14 days, but not after 87 days (there is a separate message for that)
    const fourteenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 14
    const eightySevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 87
    const now = Date.now()

    if(now < fourteenDaysAfterSubscriptionStarted || now >= eightySevenDaysAfterSubscriptionStarted) {
      condition.disable()
      return
    }

    api.addon.date = fourteenDaysAfterSubscriptionStarted / 1000
    condition.enable()
  }
  
  api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());
  
  computeCondition();
});
