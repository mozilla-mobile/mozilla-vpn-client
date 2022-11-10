(function(vpn, condition) {
if (!('controller' in vpn)) {
  return;
}

if (!('state' in vpn.controller)) {
  return;
}

condition.enable();
})
