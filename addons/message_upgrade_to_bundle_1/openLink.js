((api) => {
  if (api.env.inProduction) {
    return api.urlOpener.openUrl(
        `https://www.mozilla.org/products/vpn/#pricing?utm_medium=mozilla-vpn-client&utm_source=in-app-message&utm_content=megabundle-promotion`);
  } else {
    return api.urlOpener.openUrl(
        `https://www-dev.allizom.org/en-US/products/vpn/#pricing`);
  }
});
