// Let's use a global variable to keep track of the split-tunneling settings.
// This will be replaced with addon properties as soon as they will be
// implemented (v2.13~).
splitTunnelTutorialVpnWasOn = false;

(function(api, condition) {
api.connectSignal(api.addon, 'playingChanged', () => {
  if (!api.addon.playing) {
    return;
  }

  if (api.controller.state === api.controller.StateOff) {
    condition.disable();
    splitTunnelTutorialVpnWasOn = false;
  } else {
    condition.enable();
    splitTunnelTutorialVpnWasOn = true;
  }
});

condition.disable();
});
