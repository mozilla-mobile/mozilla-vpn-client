(function(api, condition) {
  // Show message only if within 1 week of expiring.
  let weekBeforeExpireMSecs = api.subscriptionData.expiresOn - 1000 * 60 * 60 * 24;

  if (Date.now() < api.subscriptionData.expiresOn &&
      Date.now() >= weekBeforeExpireMSecs) {
    api.addon.date = weekBeforeExpireMSecs;
    condition.enable();
  } else {
    condition.disable();
  }
})
