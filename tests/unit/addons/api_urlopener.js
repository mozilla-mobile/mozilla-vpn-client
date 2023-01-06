(function(vpn, condition) {
if (!('urlOpener' in vpn)) {
  return;
}

if (vpn.urlOpener.lastUrl !== '') {
  return;
}

vpn.urlOpener.openUrl('https://example.com');
if (vpn.urlOpener.lastUrl !== 'https://example.com') {
  return;
}

vpn.urlOpener.openLink(vpn.urlOpener.LinkContact);
if (vpn.urlOpener.lastUrl !== 'https://vpn.mozilla.org/r/vpn/contact') {
  return;
}


condition.enable();
})
