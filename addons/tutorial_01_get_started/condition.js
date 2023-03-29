(function(api, condition) {
    if(!api.featureList.get('recommendedServers').isSupported) {
        condition.enable()
        return
    }
    condition.disable()
  })