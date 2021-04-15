console.log('A');
var port = browser.runtime.connectNative('mozillavpn');
console.log(port);

port.onMessage.addListener((response) => {
  console.log('Received: ' + JSON.stringify(response));
});

port.postMessage('ping');
