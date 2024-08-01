// Disable all Android and iOS versions, and all macOS versions before 11
(function(api, condition) {
  // First check for Android...
  const isAndroid = (api.env.platform === "android");
  const isIOS = (api.env.platform === "ios");
  if (isAndroid || isIOS) {
    condition.disable();
    return;
  }

  // Then check for macOS...
  const isMacOS = (api.env.platform === "macos");
  if (isMacOS) {
    // ...then check for the minimum version - minimum is 11 for macOS
    const minVersion = 11;
    const osVersion = api.env.osVersion;
    if (!osVersion || (typeof osVersion !== "string") || osVersion.length === 0) {
      // Something unexpected happened. Enable on failure.
      condition.enable();
      return;
    }
    const majorVersionString = osVersion.split(".", 1)[0];
    const majorVersion = Number(majorVersionString);

    majorVersion >= minVersion ? condition.enable() : condition.disable();
  }

  // For all other platforms, enable the addon.
  condition.enable();
  return;
});
  