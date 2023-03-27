(function(vpn, condition) {
vpn.connectSignal(vpn.settings, 'startAtBootChanged', () => condition.enable());
})
