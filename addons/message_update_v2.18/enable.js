(function (api) {
  // "Extra" fields are used only to get a localized string.
  api.addon.composer.remove('extra_1');

  if (('updateTime' in api.settings)) {
    api.addon.date = (api.settings.updateTime.getTime() / 1000);
  }


  // Macos v2.16.0 requires a web-based update.
  if (api.env.platform === 'macos' && api.env.versionString === '2.16.0') {
    api.addon.setTitle(
        'message.message_update_v2.18.block.extra_1',
        'Download the new Mozilla VPN');
    api.addon.composer.remove('c_3');
    return;
  }

  // Windows v2.10 to v2.12 do require a web-based update,
  // with the exception on v2.11.1.
  if (api.env.platform !== 'windows' || api.env.versionString === '2.11.1') {
    api.addon.composer.remove('c_4');
    return;
  }

  const parts = api.env.versionString.split('.');

  // No idea which version we are in...
  if (parts.length < 3) {
    api.addon.composer.remove('c_4');
    return;
  }

  const version = parts.map(a => parseInt(a, 10));

  function versionCompare(a, b) {
    for (let i = 0; i < 3; ++i) {
      if (a[i] != b[i]) {
        return a[i] > b[i] ? -1 : 1;
      }
    }
    return 0;
  }

  if (versionCompare([2, 13, 0], version) >= 0 ||
    versionCompare([2, 10, 0], version) < -1) {
    api.addon.composer.remove('c_4');
    return;
  }

  api.addon.composer.remove('c_3');

  api.addon.setTitle(
      'message.message_update_v2.18.block.extra_1',
      'Download the new Mozilla VPN');
})
