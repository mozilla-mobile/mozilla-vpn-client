(function(api, condition) {
    api.connectSignal(api.featureList.get('recommendedServers'), 'supportedChanged', () => {
        if(!api.featureList.get('recommendedServers').isSupported) {
            condition.enable()
            return
        }
        condition.disable()
      });

    if(!api.featureList.get('recommendedServers').isSupported) {
        condition.enable()
        return
    }
    condition.disable()
  })