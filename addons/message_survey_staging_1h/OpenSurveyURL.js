((api) => {
  const desktop = ['linux', 'macos', 'windows'].includes(api.env.platform);
  if (desktop) {
    return api.urlOpener.openUrl(
        `https://survey.alchemer.com/s3/6897437/VPN-Lifecycle-Survey-Early-Usage-28d-Desktop?platform=__VPN_PLATFORM__&os=__VPN_OS__&version=__VPN_VERSION__`);
  }
  return api.urlOpener.openUrl(
      `https://survey.alchemer.com/s3/6897488/VPN-Lifecycle-Survey-Early-Usage-28d-Mobile?platform=__VPN_PLATFORM__&os=__VPN_OS__&version=__VPN_VERSION__`);
});
