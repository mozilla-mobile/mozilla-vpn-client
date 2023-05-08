(function(api, condition) {
function computeCondition() {
  if (['USD', 'CAD'].includes(api.subscriptionData.planCurrency)) {
    condition.enable();
    return;
  }

  condition.disable();
}

api.connectSignal(api.subscriptionData, 'changed', () => computeCondition());

computeCondition();
});
