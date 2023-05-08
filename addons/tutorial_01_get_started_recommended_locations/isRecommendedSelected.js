(function(api, condition) {
    api.connectSignal(api.settings, 'recommendedServerSelectedChanged', () => {
        if(api.settings.recommendedServerSelected === false) {
            condition.enable()
            return
        }
        condition.disable()
      });
    
    if(api.settings.recommendedServerSelected === false) {
        condition.enable()
        return
    }
    condition.disable()
  })