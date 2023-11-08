(function(api, condition) {
  function computeCondition() {

    //Enable message for users on the FPN 6 month trial plan only
    const fpnPlanId = "price_1MzNRCJNcmPzuWtRMCwUWADu"
    const subscriptionDataJson = JSON.parse(api.settings.subscriptionData)
    const subscriptionPlanId = subscriptionDataJson["plan_id"]

    subscriptionPlanId === fpnPlanId ? condition.enable() : condition.disable()
  }

  api.connectSignal(api.settings.subscriptionData, 'changed', () => computeCondition());

  computeCondition();
});
