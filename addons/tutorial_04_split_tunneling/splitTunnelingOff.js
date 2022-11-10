(function(api, condition) {
api.connectSignal(api.settings, 'protectSelectedAppsChanged', () => {
  if (!api.addon.playing) {
    if (api.settings.protectSelectedApps) {
      condition.disable();
    } else {
      condition.enable();
    }
  }
});

if (api.settings.protectSelectedApps) {
  condition.disable();
} else {
  condition.enable();
}
});
