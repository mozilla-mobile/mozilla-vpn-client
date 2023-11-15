(function(api, condition) {  
  //Show this message after 14 days, but not after 87 days (there is a separate message for that)

  //There are 2 identical (content-wise), yet separate messages because we want to give the illusion that we are 
  //re-showing the same message 87 days in (by hiding the 14 day one and showing the 87 day one) without having to
  //un-read or un-dismiss a message

  api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());

  //Slight delay to as we wait for subscriptionData to initialize
  api.setTimeout(5000, () => computeCondition());

  function isMonthlyWebPlan() {
    return api.subscriptionData.type === api.subscriptionData.SubscriptionWeb &&
           api.subscriptionData.planBillingInterval === api.subscriptionData.BillingIntervalMonthly
  }

  function computeCondition() {
    let now = Date.now()
    let fourteenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 14
    let eightySevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 87

    if (isMonthlyWebPlan) {
      if (now < fourteenDaysAfterSubscriptionStarted) {
        api.clearTimers()
        api.setTimeout(fourteenDaysAfterSubscriptionStarted - now, () => computeCondition());
        condition.disable()
      }
      else if (now >= eightySevenDaysAfterSubscriptionStarted) {
        api.clearTimers()
        api.setTimeout(eightySevenDaysAfterSubscriptionStarted - now, () => computeCondition())
        condition.disable()
      }
      else {
        api.addon.date = fourteenDaysAfterSubscriptionStarted / 1000
        condition.enable()
      }
    }
  }
});
