((api) => {
  // windows v2.10/v2.11 do require a web-based update.
  if (api.env.platform != 'windows') {
    api.navigator.requestScreen(api.navigator.ScreenUpdateRecommended);
    return;
  }

  const parts = api.env.versionString.split('.');

  // No idea which version we are in...
  if (parts.length < 3) {
    api.navigator.requestScreen(api.navigator.ScreenUpdateRecommended);
    return;
  }

  const version = parts.map(a => parseInt(a, 10));

  function versionCompare(a, b) {
    for (let i = 0; i < 3; ++i) {
      if (a[i] != b[i]) {
        return a[i] > b[i] ? -1 : 1;
      }
    }
    return 0;
  }

  if (versionCompare([2, 11, 1], version) >= 0 ||
      versionCompare([2, 10, 0], version) < -1) {
    api.navigator.requestScreen(api.navigator.ScreenUpdateRecommended);
    return;
  }

  api.urlOpener.openLink(api.urlOpener.LinkUpdate);
});
