(function(api) {
  function versionCompare(a, b) {
    for (let i = 0; i < 3; ++i) {
      if (a[i] != b[i]) {
        return a[i] > b[i] ? -1 : 1;
      }
    }
    return 0;
  }

  const parts = api.env.versionString.split('.');

  const version = parts.map(a => parseInt(a, 10));

  //Post 2.16 API
  if (versionCompare([2, 16, 0], version) >= 0) {
	api.navigator.requestScreen(api.vpn.ScreenGetHelp);
    return;
  }
  //Pre 2.16 API
  else {
  	api.navigator.requestScreen(api.navigator.ScreenGetHelp);
  	return;
  }
})
