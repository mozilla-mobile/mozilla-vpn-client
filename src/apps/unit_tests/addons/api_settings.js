(function(api, condition) {
if (!('settings' in api)) {
  return;
}

if (api.settings.addonApiSetting) {
  return;
}

api.settings.addonApiSetting = true;

if (!api.settings.addonApiSetting) {
  return;
}

condition.enable();
})
