((api) => {
  api.navigator.requestScreen(
      'vpn' in api ? api.vpn.ScreenUpdateRecommended :
                     api.navigator.ScreenUpdateRecommended);
});
