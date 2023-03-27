(function(api, condition) {
if (!('env' in api)) {
  return;
}

if (api.env.inProduction) {
  return;
}

if (typeof (api.env.versionString) !== 'string' ||
    api.env.versionString.length === 0) {
  return;
}

if (typeof (api.env.buildNumber) !== 'string' ||
    api.env.buildNumber.length === 0) {
  return;
}

if (typeof (api.env.osVersion) !== 'string' || api.env.osVersion.length === 0) {
  return;
}

if (typeof (api.env.architecture) !== 'string' ||
    api.env.architecture.length === 0) {
  return;
}

if (typeof (api.env.platform) !== 'string' || api.env.platform.length === 0) {
  return;
}

condition.enable();
})
