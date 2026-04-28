(function(api, condition) {
if (!('featureList' in api)) {
  return;
}

if (!api.featureList.get('alwaysPort53')) {
  return;
}

if (api.featureList.get('alwaysPort53').isSupported) {
  return;
}

api.featureList.toggle('alwaysPort53');

if (!api.featureList.get('alwaysPort53').isSupported) {
  return;
}

condition.enable();
})
