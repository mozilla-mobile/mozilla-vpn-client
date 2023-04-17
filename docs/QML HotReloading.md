

# Using QML hot reloading. 
The Client has roudimentary support for Live-Reloading QML. While developing you can replace the loaded QML and have the Client automatically reload the Current Screen. 

## Usage (Desktop): 
- Have MozillaVPN open and in Dev Mode. 
- run `npm run qml-reload` or `node /tools/qml_reload/hot.js`
- Edit any qml file. 

You can exit using `CMD+C` - any changes will be reset. 

## Usage (Android)
- Have MozillaVPN open and in Dev Mode. 
- Have the Device connected via ADB
- forward local network traffic: 
- Forward a websocket connection to the device `adb forward tcp:8765 tcp:8765`
- Let the device access the served files `adb reverse tcp:8888 tcp:8888` 
- run `npm run qml-reload` or `node /tools/qml_reload/hot.js`
- Edit any qml file. 

