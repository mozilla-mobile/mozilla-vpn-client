(function(api, condition) {
api.connectSignal(api.addon, 'playingChanged', () => {
  if (!api.addon.playing) {
    return;
  }

  if (splitTunnelTutorialVpnWasOn) {
    condition.enable();
  } else {
    condition.disable();
  }
});

condition.disable();
});
