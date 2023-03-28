(function(api, condition) {
api.connectSignal(
    api.settings, 'addonApiSettingChanged', () => condition.enable());
})
