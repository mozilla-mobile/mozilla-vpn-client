(function(api, condition) {
if (!('urlOpener' in api)) {
  return;
}

api.urlOpener.openUrl('https://example.com');
if (api.urlOpener.lastUrl !== 'https://example.com') {
  return;
}

api.urlOpener.openUrlLabel('aa');
if (api.urlOpener.lastUrl !== 'http://foo.bar') {
  return;
}

condition.enable();
})
