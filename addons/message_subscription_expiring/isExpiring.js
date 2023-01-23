(function(api, condition) {
  // Show message only if within 1 week of expiring.
  const weekBeforeExpireMSecs = api.subscriptionData.expiresOn - 1000 * 60 * 60 * 24 * 7;
  const subscriptionExpiry = api.subscriptionData.expiresOn;
  const now = Date.now();

  if (now < subscriptionExpiry && now >= weekBeforeExpireMSecs) {
    api.addon.date = weekBeforeExpireMSecs;
    condition.enable();
  } else {
    condition.disable();
  }
});
