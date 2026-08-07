// show for:
// 2.31.9 and earlier: macOS and Linux
// 2.33.9 and earlier: Windows and iOS
// 2.34.9 and earlier: Android

// Disable all Android and iOS versions, and all macOS versions before 11
(function(api, condition) {

function versionCompare(a, b) {
  for (let i = 0; i < 3; ++i) {
    if (a[i] != b[i]) {
      return a[i] > b[i] ? -1 : 1;
    }
  }
  return 0;
}

function computeCondition() {
  const parts = api.env.versionString.split('.');
  const version = parts.map(a => parseInt(a, 10));

  const isMacOS = (api.env.platform === 'macos');
  const isLinux = (api.env.platform === 'linux');
  if (isMacOS || isLinux) {
    // Post 2.32
    if (versionCompare([2, 32, 0], version) >= 0) {
      condition.disable();
      return;
    }
  }

  const isIOS = (api.env.platform === 'ios');
  const isWindows = (api.env.platform === 'windows');
  if (isWindows || isIOS) {
    // Post 2.34
    if (versionCompare([2, 34, 0], version) >= 0) {
      condition.disable();
      return;
    }
  }

  // Covers Android
  // Additionally, we fail restrictive - any unknown or poorly-formed platform
  //   must be on 2.35 or later (guaranteed okay on all platforms) to not see
  //   message
  // Post 2.35
  if (versionCompare([2, 35, 0], version) >= 0) {
    condition.disable();
    return;
  }

  // We've confirme we're in a version that may require this addon. Now check
  // the date.
  let now = Date.now();
  let startTime = 1791478800000  // 10am Pacific on Oct 8 2026

  if (now < startTime) {
    condition.disable()
    // Set callback - if app isn't closed, we need to turn on this addon at the
    // appropriate time
    api.setTimedCallback(startTime, () => computeCondition());
  }
  else {
    condition.enable()
  }
}

computeCondition();
});
