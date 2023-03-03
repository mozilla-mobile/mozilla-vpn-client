/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


const WebSocket = require('ws');
const chokidar = require('chokidar');

var currentPath = process.cwd();
const ws_url = 'ws://localhost:8765';

const websocket = new WebSocket(ws_url);

websocket.on('open', function() {
  const watcher = chokidar.watch('.', {
    ignored: /(^|[\/\\])\../,
    persistent: true,
    ignoreInitial: true,
    awaitWriteFinish: {stabilityThreshold: 200, pollInterval: 100}
  });
  watcher.on('all', function(event, path) {
    if (event === 'add' || event === 'change') {
      if (path.endsWith('.qml')) {
        const relPath = path.replace(/\\/g, '/');

        const url = `file://${currentPath}/${relPath}`;
        websocket.send(`live_reload ${url}`);
      }
    }
  });
});

// When the WebSocket connection is closed, log it to the console
websocket.on('close', function() {
  console.log('Websocket Closed')
  process.exit(0)
});

// When an error occurs, log it to the console
websocket.on('error', function(error) {
  console.error('WebSocket error:', error);
  process.exit(0)
});

// TODO: Theoretically we can serve this via http:
// however i found that the first load was always screwed up lol
//
// const server = http.createServer(function (req, res) {
//   const url = req.url === '/' ? '/index.html' : req.url;
//   const filePath = '.' + url;
//   try {
//     const stream = fs.createReadStream(filePath);
//     stream.pipe(res);
//   } catch (error) {
//     res.end();
//   }
// });
// server.listen(http_port, function() {
//   console.log('HTTP server serving at port', http_port);
// })