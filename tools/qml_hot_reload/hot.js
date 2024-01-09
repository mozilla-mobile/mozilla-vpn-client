/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { InspectorWebsocketClient, DEFAULT_URL } from '@mozillavpn/inspector'
import  commandLineArgs  from 'command-line-args'
import * as chokidar from 'chokidar'

import { ADB } from 'appium-adb'

import * as http from 'node:http'
import * as fs from 'node:fs'
import * as path from 'node:path'


(async () => {
  const optionDefinitions = [
    { name: "remote", alias: "r", type: Boolean, defaultValue: false },
    { name: "adb", alias: "a", type: Boolean, defaultValue: false },
  ];
  const http_port = 8888;
  const options = commandLineArgs(optionDefinitions);

  async function forwardADBPorts() {
    console.log("Enable Android ADB forwarding");
    const adb = await ADB.createADB();
    const hasEmulator = await adb.isEmulatorConnected();
    const hasDevice = await adb.isDeviceConnected();
    if (!(hasEmulator || hasDevice)) {
      throw new Error("No device found to forward to!");
    }
    await adb.forwardPort("8765", "8765");
    await adb.reversePort(http_port, http_port);
  }

  var currentPath = process.cwd();

  if (options.adb === true) {
    await forwardADBPorts();
  }
  const client = new InspectorWebsocketClient(DEFAULT_URL);
  console.log(`Connecting to Client at ${DEFAULT_URL}`);

  const qtWebChannel = await new Promise(resolve => client.qWebChannel.subscribe((c)=>{
    if(c){
      resolve(c);
    }
  }));
  console.log(`Connected to Client!`);
  console.log(
        `Controls: \n r \t \t - Force Reload the whole window \n control+c \t - exit`);

  const watcher = chokidar.watch(`.`, {
      ignored: /(^|[\/\\])\../,
      persistent: true,
      ignoreInitial: true,
      awaitWriteFinish: { stabilityThreshold: 200, pollInterval: 100 },
  });
    watcher.on("all", function (event, changedPath) {
      console.log("hiii")
      if (event === "add" || event === "change") {
        if (changedPath.endsWith(".qml")) {          
          let url = `file://${currentPath}/${changedPath}`;
          if (options.remote) {
            url = `http://0.0.0.0:${http_port}/${changedPath}`;
          }
          console.log(`Announcing ${url}`);
          qtWebChannel.objects.inspectorHotReloader.annonceReplacedFile(url.replace("\\","/"));
        }
      }
  });
  process.on("SIGINT", function () {
    console.log("Unregistering all reloads...");
    qtWebChannel.objects.inspectorHotReloader.resetAllFiles();
    process.exit();
  });

  var stdin = process.stdin;
  stdin.setRawMode(true);
  stdin.resume();
  stdin.setEncoding('utf8');

  stdin.on('data', function(key) {
    // ctrl-c ( end of text )
    if (key === '\u0003') {
      qtWebChannel.objects.inspectorHotReloader.resetAllFiles();
      process.exit();
    }
    if (key === 'r') {
      qtWebChannel.objects.inspectorHotReloader.reloadWindow();
      console.log('Requesting App Reload')
    }
  });

  if (options.remote) {
    const server = http.createServer(function (req, res) {
      console.log(`(Server) /t Requests for ${req.url}`);

      console.log(req.rawHeaders);
      try {
        const url = req.url === "/" ? "/index.html" : req.url;
        const filePath = "." + url;
        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
      } catch (error) {
        console.log(error);
        res.end();
      }
    });
    server.listen(http_port, function () {
      console.log("HTTP server serving hot qml at 0.0.0.0:" + http_port);
    });
  }
})();
