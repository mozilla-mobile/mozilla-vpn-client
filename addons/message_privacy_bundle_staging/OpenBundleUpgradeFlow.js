((api) => {
  api.urlOpener.openUrl(
      api.env.inProduction ?
          `https://vpn.mozilla.org/r/vpn/upgradeToPrivacyBundle` :
          `https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net/r/vpn/upgradeToPrivacyBundle`);
});
