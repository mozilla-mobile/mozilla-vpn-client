(function(vpn, condition) {
if (!('subscriptionData' in vpn)) {
  return;
}

condition.enable();
})
