var port = browser.runtime.connectNative('mozillavpn');

port.onMessage.addListener((response) => {
  console.log('Received: ' + JSON.stringify(response));
});

setInterval(() => {
  port.postMessage({t: 'servers'});
  port.postMessage({t: 'status'});
  port.postMessage({t: 'disabled_apps'});
}, 1000);
