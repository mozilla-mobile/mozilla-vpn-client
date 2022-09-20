((api) => {
  const utmParams =
      '?utm_source=in-app-message&utm_medium=vpn-client&utm_campaign=bundle-promo';
  api.urlOpener.openUrl(
      api.env.inProduction ?
          `https://vpn.mozilla.org/r/vpn/upgradeToPrivacyBundle${utmParams}` :
          `https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net/r/vpn/upgradeToPrivacyBundle${
              utmParams}`);
});
