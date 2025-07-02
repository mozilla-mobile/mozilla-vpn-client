((api) => {
  if (api.env.inProduction) {
    return api.urlOpener.openUrl(
        `https://www.mozilla.org/products/vpn/?utm_medium=mozilla-vpn-client&utm_source=in-app-message&utm_content=megabundle-promotion&geo=US&entrypoint_experiment=vpn-landing-bundle-promo&entrypoint_variation=c#pricing`);
  } else {
    return api.urlOpener.openUrl(
        `https://www-dev.allizom.org/products/vpn/?utm_medium=mozilla-vpn-client&utm_source=in-app-message&utm_content=megabundle-promotion&geo=US&entrypoint_experiment=vpn-landing-bundle-promo&entrypoint_variation=c#pricing`);
  }
});
