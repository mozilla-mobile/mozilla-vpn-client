(function(vpn, condition) {
if (!('env' in vpn)) {
  return;
}

if (vpn.env.inProduction) {
  return;
}

if (typeof (vpn.env.versionString) !== 'string' ||
    vpn.env.versionString.length === 0) {
  return;
}

if (typeof (vpn.env.buildNumber) !== 'string' ||
    vpn.env.buildNumber.length === 0) {
  return;
}

if (typeof (vpn.env.osVersion) !== 'string' || vpn.env.osVersion.length === 0) {
  return;
}

if (typeof (vpn.env.architecture) !== 'string' ||
    vpn.env.architecture.length === 0) {
  return;
}

if (typeof (vpn.env.platform) !== 'string' || vpn.env.platform.length === 0) {
  return;
}

condition.enable();
})
