(function (api) {
  // Extra_1 and extra_2 are used only to have a localized string.
  api.addon.composer.remove('extra_1');
  api.addon.composer.remove('extra_2');

  if (('updateTime' in api.settings)) {
    api.addon.date = (api.settings.updateTime.getTime() / 1000);
  }

  // Windows v2.10 to v2.12 do require a web-based update,
  // with the exception on v2.11.1.
  if (api.env.platform !== 'windows' || api.env.versionString === '2.11.1') {
    api.addon.composer.remove('c_4');
    api.addon.composer.remove('c_2b');
    return;
  }

  const parts = api.env.versionString.split('.');

  // No idea which version we are in...
  if (parts.length < 3) {
    api.addon.composer.remove('c_4');
    api.addon.composer.remove('c_2b');
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
    api.addon.composer.remove('c_2b');
    return;
  }

  api.addon.composer.remove('c_2');
  api.addon.composer.remove('c_3');

  api.addon.setTitle(
    'message.message_update_v2.15.block.extra_1', 'Download Mozilla VPN 2.15')
  api.addon.setSubtitle(
    'message.message_update_v2.15.block.extra_2',
    'We’ve released an updated version of Mozilla VPN! Download it today to get the newest features and bug fixes:');
})
