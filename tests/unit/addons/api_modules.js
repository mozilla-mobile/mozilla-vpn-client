(function(api, condition) {
if (!('modules' in api)) {
  return;
}

if (!('vpn' in api.modules)) {
  return;
}

if (!('controller' in api.modules['vpn'])) {
  return;
}

if (!('state' in api.modules['vpn'].controller)) {
  return;
}

condition.enable();
})
