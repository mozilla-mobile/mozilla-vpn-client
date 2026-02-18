// Only enable for macOS 11 and 12, as 2.32.0 is the final release those
// machines can receive.

(function(api, condition) {
// Confirm it's macOS...
const isMacOS = (api.env.platform === 'macos');
if (!isMacOS) {
  // This shouldn't be possible. Do not show.
  condition.disable();
  return;
}

// Platform filtering is done in manifest.json, this only checks version
const osVersion = api.env.osVersion;
if (!osVersion || (typeof osVersion !== 'string') || osVersion.length === 0) {
  // Something unexpected happened. Disable on failure.
  condition.disable();
  return;
}
const majorVersionString = osVersion.split('.', 1)[0];
const majorVersion = Number(majorVersionString);

// Only enable for macOS 11 or 12
if (majorVersion === 11 || majorVersion === 12) {
  condition.enable();
} else {
  condition.disable();
}
});
