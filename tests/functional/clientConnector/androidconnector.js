/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


const { ADB } = require('appium-adb')

const path = require('node:path');
const { existsSync, writeFileSync } = require('node:fs');
const guardian = require('../servers/guardian');
const fxa = require('../servers/fxa');

const websocket = require('websocket').w3cwebsocket;


const maybe_adb = ADB.createADB();


let client;

module.exports = {
  async connect(options = {
    "guardian": new URL(),
    "fxa":new URL(),
    "fxa":new URL(),
    "addon": new URL(),
    "benchmark":new URL(),
    "captivePortal":new URL(),
    }, 
      onopen = async ()=>{}, // Returns a promise that should be awaited ? 
      onclose =()=>{},  // Callback, should be called once we have closed a connection
      onmessage=()=>{} // Callback, should be called when we recieve a message!
      )
    {

    /**
     * Let's assume nothing: 
     * Check if a suitable ADB device is present, if so use that 
     * if not, let's search for an emulator and turn that up. 
     * 
     * Once we have an ADB connection: 
     * - Forward the websocket port from the Phone -> localhost 
     * - Forward the servers from localhost -> Phone
     * - Start the APP to launch 
     * - PRAY!
     * - Establish a websocket connection
     * - Run test. 
     */
    const adb = await maybe_adb; 
    const hasEmulator = await adb.isEmulatorConnected()
    const hasDevice = await adb.isDeviceConnected();

    console.log({
      hasEmulator,hasDevice
    })
    if (!(hasEmulator || hasDevice)){
      throw new Error("No device found to run tests on!")
    }
    // Find the APK to use: 
    // If there is an ENV MVPN_FORCE_APK with a path, use that. 
    let target = process.env.MVPN_FORCE_APK
    if(!existsSync(target)){
      // If not: Get the Devices ABI and 
      // check if a build for that in .tmp is existing. 
      const ABI = await adb.shell("getprop ro.product.cpu.abi")
      const currentPath = process.cwd();
      const apk_folder = path.join(currentPath,".tmp","src","android-build","build","outputs","apk","debug");
      const apk = path.join(apk_folder,`android-build-${ABI}-debug.apk`)
      if(!existsSync(apk)){
        throw new Error("No file provided and auto-detected file: "+apk+" does not exist.")
      }
      target = apk;
    }
    console.log(`Target APK ${target}`)

    while(await adb.isScreenLocked()){
        console.error("Your screen is locked!")
    }
    
    //console.log("Installing APK")
    /* 
    await adb.install(target);

    // Allow the apk to access all of storage
    adb.grantPermission("org.mozilla.firefox.vpn.debug","android.permission.MANAGE_EXTERNAL_STORAGE")

    // We now have to make sure the app's settings are correct:
    // We need to be in stage mode and have guardian && addon's set. 
    //const file = await this.pullSettingFile();
    let settings = {}
    // Modify the current settings so that the functional tests can connect!
    settings["addon/customServerAddress"] = options.addon
    settings["addon/customServer"] = true
    settings["localhostRequestOnly"] = true
    settings["stagingServerAddress"]= options.guardian
    settings["stagingServer"]= true
    await this.pushSettingsFile(JSON.stringify(settings));

    */ 
    
    // Now we need to forward some port's so i don't have to figure out the network
    // so that the 2 devices can talk .__."
    await adb.reversePort(guardian.port,guardian.port)
    await adb.reversePort(fxa.port,fxa.port)
    await adb.forwardPort("8765", "8765");

    //await adb.forwardPort(addon.port,addon.port)
    
    
    // Okay cross fingers, let's boot that thing. 
    await adb.shell("am start -n org.mozilla.firefox.vpn.debug/org.mozilla.firefox.vpn.qt.VPNActivity")

    return await new Promise(resolve => {
      client = new websocket(`ws://127.0.0.1:8765/`, '');

      client.onopen = async () => {
        await onopen();
        resolve(true);
      };

      client.onclose = () => onclose();
      client.onerror = (e) => {
        resolve(false);
      }

      client.onmessage = data => onmessage(JSON.parse(data.data));
    });
  },

  async close() {
    return client.close();
  },

  async send(msg) {
    return client.send(msg);
  },
}
