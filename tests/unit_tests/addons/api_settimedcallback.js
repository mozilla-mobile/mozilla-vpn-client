(function(api, condition) {
    api.log("settimedcallback started");
    api.setTimedCallback(1000, () => {
        api.log("settimedcallback fired");
        condition.enable();
    });
});
