---
sidebar_position: 3
---

# QML Hot Reloading. 
The Client has simple support for live reloading QML file. 

https://user-images.githubusercontent.com/9611612/222613103-edca265c-f489-4762-afa7-22d344d62c19.mov

## Usage (Desktop): 
- Have MozillaVPN open and in Dev Mode. 
- Compile the hotreloader: 
- cmake --build <your build directory> --target hotreloader
- run <your build directory>/tools/tools/qml_hot_reload/hotreloader
- Edit any qml file. 

You can exit using `CMD+C` - any changes will be reset. 


