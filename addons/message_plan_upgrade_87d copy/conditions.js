(function(api, condition) {
  function computeCondition() {

    //Do not show message to non-web (eg IAP) or non-monthly (eg yearly) subscribers
    if (api.subscriptionData.type !== api.subscriptionData.SubscriptionWeb &&
        api.subscriptionData.planBillingInterval !== api.subscriptionData.BillingIntervalMonthly) {
      condition.disable()
      return
    }

    //Show this message after 87 days
    const eightySevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 87
    const now = Date.now()

    if(now < eightySevenDaysAfterSubscriptionStarted) {
      condition.disable()
      return
    }

    api.addon.date = eightySevenDaysAfterSubscriptionStarted / 1000
    condition.enable()
  }
  
  api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());
  
  computeCondition();
});
