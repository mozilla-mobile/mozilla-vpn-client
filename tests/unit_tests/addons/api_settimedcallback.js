(function(api, condition) {
    api.setTimedCallback(1000, () => condition.enable());
});
