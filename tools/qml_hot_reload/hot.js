/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

(async () => {
  // Load .env
  require("dotenv").config();
  const commandLineArgs = require("command-line-args");

  const optionDefinitions = [
    { name: "adb", alias: "a", type: Boolean, defaultValue: false },
  ];
  const options = commandLineArgs(optionDefinitions);

  console.log(options);

  async function forwardADBPorts() {
    console.log("Enable Android ADB forwarding");
    const { ADB } = require("appium-adb");
    const adb = await ADB.createADB();
    const hasEmulator = await adb.isEmulatorConnected();
    const hasDevice = await adb.isDeviceConnected();
    if (!(hasEmulator || hasDevice)) {
      throw new Error("No device found to forward to!");
    }
    await adb.forwardPort("8765", "8765");
  }

  const http = require("http");
  const fs = require("fs");
  const WebSocket = require("ws");
  const chokidar = require("chokidar");

  var currentPath = process.cwd();
  const ws_url = "ws://127.0.0.1:8765";

  if (options.adb === true) {
    await forwardADBPorts();
  }
  const websocket = new WebSocket(ws_url, {
    origin: "https://mozilla-mobile.github.io"
  });
  console.log(`Connecting to Client at ${ws_url}`);

  websocket.on("open", function () {
    console.log(`Connected to Client!`);
    console.log(
        `Controls: \n r \t \t - Force Reload the whole window \n control+c \t - exit`);

    console.log(`Watching: ${currentPath}`);

    const watcher = chokidar.watch(".", {
      ignored: (path, stats) => stats?.isFile() && !path.endsWith('.qml'), // only watch QML files
      //ignored: /(^|[\/\\])\../,
      persistent: true,
      ignoreInitial: true,
      awaitWriteFinish: { stabilityThreshold: 200, pollInterval: 100 },
    });
    watcher.on("all", function (event, path) {
      if (event === "add" || event === "change") {
        if (path.endsWith(".qml")) {
          const relPath = path.replace(/\\/g, "/");
          let url = `file://${currentPath}/${relPath}`;
          console.log(`Announcing ${url}`);
          websocket.send(`live_reload ${url}`);
        }
      }
    });
  });

  // When the WebSocket connection is closed, log it to the console
  websocket.on("close", function () {
    console.log("Websocket Closed");
    process.exit(0);
  });

  // When an error occurs, log it to the console
  websocket.on("error", function (error) {
    console.error("WebSocket error:", error);
    process.exit(0);
  });

  process.on("SIGINT", function () {
    console.log("Unregistering all reloads...");
    websocket.send(`reset_live_reload`);
    process.exit();
  });

  var stdin = process.stdin;
  stdin.setRawMode(true);
  stdin.resume();
  stdin.setEncoding('utf8');

  stdin.on('data', function(key) {
    // ctrl-c ( end of text )
    if (key === '\u0003') {
      websocket.send(`reset_live_reload`);
      process.exit();
    }
    if (key === 'r') {
      websocket.send(`reload_window`);
      console.log('Requesting App Reload')
    }
  });
})();
