(function(api, condition) {
// Let's reset the disabled app list to have the first one item in the list
// always disabled.  Because we are doing at the beginning of the tutorial, the
// settings-rollback will take care of restoring the previous config.
api.connectSignal(api.addon, 'playingChanged', () => {
  if (api.addon.playing) {
    api.settings.vpnDisabledApps = [];
  }
});

condition.enable();
});
