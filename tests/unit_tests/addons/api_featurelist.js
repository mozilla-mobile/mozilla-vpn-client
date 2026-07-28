(function(api, condition) {
if (!('featureList' in api)) {
  return;
}

if (!api.featureList.get('showRotateIPAddressButton')) {
  return;
}

if (api.featureList.get('showRotateIPAddressButton').isSupported) {
  return;
}

api.featureList.toggle('showRotateIPAddressButton');

if (!api.featureList.get('showRotateIPAddressButton').isSupported) {
  return;
}

condition.enable();
})
