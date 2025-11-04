(function(api, condition) {
if (!('env' in api)) {
  api.log("env not found in api");
  return;
}

api.log("env.inProduction:" + api.env.inProduction);
if (api.env.inProduction) {
  return;
}

api.log("env.versionString:" + api.env.versionString);
if (typeof (api.env.versionString) !== 'string' ||
    api.env.versionString.length === 0) {
  return;
}

api.log("env.buildNumber:" + api.env.buildNumber);
if (typeof (api.env.buildNumber) !== 'string' ||
    api.env.buildNumber.length === 0) {
  return;
}

api.log("env.osVersion:" + api.env.osVersion);
if (typeof (api.env.osVersion) !== 'string' || api.env.osVersion.length === 0) {
  return;
}

api.log("env.architecture:" + api.env.architecture);
if (typeof (api.env.architecture) !== 'string' ||
    api.env.architecture.length === 0) {
  return;
}

api.log("env.platform:" + api.env.platform);
if (typeof (api.env.platform) !== 'string' || api.env.platform.length === 0) {
  return;
}

condition.enable();
})
