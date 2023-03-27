(function(vpn, condition) {
if (!('featureList' in vpn)) {
  return;
}

if (!vpn.featureList.get('testFeatureAddonApi')) {
  return;
}

if (vpn.featureList.get('testFeatureAddonApi').isSupported) {
  return;
}

vpn.featureList.toggle('testFeatureAddonApi');

if (!vpn.featureList.get('testFeatureAddonApi').isSupported) {
  return;
}

condition.enable();
})
