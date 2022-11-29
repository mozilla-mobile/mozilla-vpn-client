(function(api) {
if (!('updateTime' in api.settings)) {
  api.addon.date = 1668020390;
  return;
}

api.addon.date = (api.settings.updateTime.getTime() / 1000);
})
