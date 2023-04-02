let enabled = false;
(function(api) {
if (enabled) {
  console.log('Disable...');
  api.addon.parent.disable(api.addon.TypeReplacer);
  console.log('Disabled');
} else {
  console.log('Enabling...');
  api.addon.parent.enable(api.addon.TypeReplacer);
  console.log('Enabled');
}

enabled = !enabled;
})
