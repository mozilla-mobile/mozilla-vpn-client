(function(api) {
if (('updateTime' in api.settings)) {
  api.addon.date = (api.settings.updateTime.getTime() / 1000);
}
})
