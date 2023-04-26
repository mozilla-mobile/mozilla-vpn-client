((api) => {  
  switch(api.subscriptionData.type) {
    case api.subscriptionData.SubscriptionApple:
      api.urlOpener.openUrlLabel("subscriptionIapApple");
      break;
    case api.subscriptionData.SubscriptionGoogle:
      api.urlOpener.openUrlLabel("subscriptionIapGoogle");
      break;
    case api.subscriptionData.SubscriptionWeb:
      api.urlOpener.openUrlLabel("subscriptionFxa");
      break;
    default:
      api.urlOpener.openUrlLabel("account");
      break;
  }
});
