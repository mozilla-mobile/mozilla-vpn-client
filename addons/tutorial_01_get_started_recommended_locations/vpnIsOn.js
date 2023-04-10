(function(api, condition) {
  api.connectSignal(api.addon, 'playingChanged', () => {
    if (!api.addon.playing) {
      return;
    }

    if (api.controller.state === api.controller.StateOff) {
      condition.disable();
      return;
    }

      condition.enable();
  });

  if (api.controller.state === api.controller.StateOff) {
    condition.disable();
    return;
  }

  condition.disable();
});
