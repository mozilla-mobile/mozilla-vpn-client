((api, condition) => {
  if (!api.featureList.get('bundleUpgrade').isSupported) {
    api.featureList.toggle('bundleUpgrade');
  }
  condition.enable();
});