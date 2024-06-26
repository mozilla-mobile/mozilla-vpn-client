// Disable on iOS 13 and earlier
(function(api, condition) {
  // First check for iOS...
  const isIOS = (api.env.platform === "ios");
  if (!isIOS) {
    condition.enable();
    return;
  }

  // ...then check for iOS 14 or later
  const minVersion = 14;
  const osVersion = api.env.osVersion;
  if (!osVersion || (typeof osVersion !== "string") || osVersion.length === 0) {
    // Something unexpected happened. Enable on failure.
    condition.enable();
    return;
  }
  const majorVersionString = osVersion.split(".", 1)[0];
  const majorVersion = Number(majorVersionString);

  majorVersion >= minVersion ? condition.enable() : condition.disable();
});
  