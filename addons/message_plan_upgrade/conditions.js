(function(api, condition) {
  function computeCondition() {

    //Do not show message to non-web (eg IAP) or non-monthly (eg yearly) subscribers
    if (api.subscriptionData.type !== api.subscriptionData.SubscriptionWeb &&
        api.subscriptionData.planBillingInterval !== api.subscriptionData.BillingIntervalMonthly) {
      condition.disable()
      return
    }

    //Show this message after 14 days, and again after 87 days
    const sevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 7
    const eightySevenDaysAfterSubscriptionStarted = api.subscriptionData.createdAt + 1000 * 60 * 60 * 24 * 87
    const now = Date.now()

    if(now < sevenDaysAfterSubscriptionStarted) {
      condition.disable()
      return
    }
    else if(now >= sevenDaysAfterSubscriptionStarted && now < eightySevenDaysAfterSubscriptionStarted) {
      api.addon.date = sevenDaysAfterSubscriptionStarted / 1000
    }
    else if(now >= eightySevenDaysAfterSubscriptionStarted) {
      api.addon.date = eightySevenDaysAfterSubscriptionStarted / 1000
    }

    //If the message was previously read or dismissed, mark as unread and undismissed
    //These values align with the MessageStatus in addonmessage.cpp
    //0 = MessageStatus::Received
    api.addon.isRead = 0

    condition.enable()
  }
  
  api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());
  
  computeCondition();
});
