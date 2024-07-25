// Disable on iOS 13 and earlier, all Android versions, and all macOS versions before 11
(function(api, condition) {
  // First check for Android...
  const isAndroid = (api.env.platform === "android");
  if (isAndroid) {
    condition.disable();
    return;
  }

  // Then check for iOS or macOS...
  const isIOS = (api.env.platform === "ios");
  const isMacOS = (api.env.platform === "macos");
  if (isIOS || isMacOS) {
    // ...then check for the minimum version - minimum is 14 for iOS, 11 for macOS
    const minVersion = isIOS ? 14 : 11;
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
  