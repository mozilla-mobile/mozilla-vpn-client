(function(vpn, condition) {
if (!('settings' in vpn)) {
  return;
}

if (vpn.settings.postAuthenticationShown) {
  return;
}

vpn.settings.postAuthenticationShown = true;

if (!vpn.settings.postAuthenticationShown) {
  return;
}

condition.enable();
})
