(function(api, condition) {  
  //Show this message after 14 days, and resurface after 87 days 

  //This will run when TaskaGetSubscriptionDetails returns something different than what is loaded from setting 
  //(often from signing in/out but can be any change in the subscription)
  api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());

  //This will run on app launch for signed in users when there subscription data is loaded into memory from settings
  api.connectSignal(api.subscriptionData, 'initialized', () => computeCondition());

  //This is for already running clients that receive this addon while the client is launched and signed in
  //(Because these users may not see a change in subscriptionData for a long time)
  computeCondition()

  function isMonthlyWebPlan() {
    return api.subscriptionData.type === api.subscriptionData.SubscriptionWeb &&
           api.subscriptionData.planBillingInterval === api.subscriptionData.BillingIntervalMonthly
  }

  function computeCondition() {
    //subscriptionData not initalized - return
    if (api.subscriptionData.createdAt <= 0) return

    let now = Date.now()
    let fourteenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 14
    let eightySevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 87

    if (isMonthlyWebPlan()) {
      //Less than 14 days into the subscription, don't show message
      if (now < fourteenDaysAfterSubscriptionStarted) {
        api.setTimedCallback(fourteenDaysAfterSubscriptionStarted - now, () => computeCondition());
        condition.disable()
      }
      //Between 14 and 87 days into the subscription, show message
      else if (now >= fourteenDaysAfterSubscriptionStarted && now < eightySevenDaysAfterSubscriptionStarted) {
        api.setTimedCallback(eightySevenDaysAfterSubscriptionStarted - now, () => computeCondition())
        api.addon.date = fourteenDaysAfterSubscriptionStarted / 1000
        condition.enable()
      }
      // After 87 days into the subscription, re-surface the message (undimiss, unread, updating timestamp)
      else if (now >= eightySevenDaysAfterSubscriptionStarted) {
        api.addon.date = eightySevenDaysAfterSubscriptionStarted / 1000
        api.addon.resetMessage()
        condition.enable()
      }
    }
  }
});
