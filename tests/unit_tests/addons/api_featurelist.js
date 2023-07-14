(function(api, condition) {
if (!('featureList' in api)) {
  return;
}

if (!api.featureList.get('testFeatureAddonApi')) {
  return;
}

if (api.featureList.get('testFeatureAddonApi').isSupported) {
  return;
}

api.featureList.toggle('testFeatureAddonApi');

if (!api.featureList.get('testFeatureAddonApi').isSupported) {
  return;
}

condition.enable();
})
