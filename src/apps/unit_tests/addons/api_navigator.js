(function(vpn, condition) {
if (!('navigator' in vpn)) {
  return;
}

condition.enable();
})
