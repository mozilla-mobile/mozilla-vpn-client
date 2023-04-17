# Using QML hot reloading. 
The Client has simple support for live reloading QML file. 

https://user-images.githubusercontent.com/9611612/222613103-edca265c-f489-4762-afa7-22d344d62c19.mov

## Usage (Desktop): 
- Have MozillaVPN open and in Dev Mode. 
- run `npm run qml-reload` or `node /tools/qml_reload/hot.js`
- Edit any qml file. 

You can exit using `CMD+C` - any changes will be reset. 

## Usage (Android)
- Have MozillaVPN open and in Dev Mode. 
- Have the Device connected via ADB
- run `node /tools/qml_reload/hot.js --remote --adb`
- Edit any qml file. 

