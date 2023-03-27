(function(vpn, condition) {
if (!('urlOpener' in vpn)) {
  return;
}

vpn.urlOpener.openUrl('https://example.com');
if (vpn.urlOpener.lastUrl !== 'https://example.com') {
  return;
}

vpn.urlOpener.openUrlLabel('aa');
if (vpn.urlOpener.lastUrl !== 'http://foo.bar') {
  return;
}

condition.enable();
})
