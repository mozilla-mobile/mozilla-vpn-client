# Using Dev-Containers 

Dev-Containers are a small experiment, they allow you to run our CI docker Images locally. 

## Requirements: 
- Node
- A Docker compatible Container Engine: 
    - docker, podman, colima, rancher) 
    - MacOS users see Q/A
    - VS-Code Extension: [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
    
 

## Example Build using CLI only: 
```bash 
# Make Sure the utils are ready.
npm install
[...]
# Now build the Docker Image- This will build the taskcluster image locally. 
npm run android_arm64:devcontainer:build
# Note: this will take a while [...]

# You now have your local repository a new image called
# cuddlebuild:android-arm64 
# Let's create a container: 
npm run android_arm64:devcontainer:up

# The Container is running, therefore we can now call into it. 
# Let's run the default android build script inside the container from the terminal
npm run android_arm64:build
# If all run sucessfully, you should now have on your machine!
ls .tmp/src/android-build/build/outputs/apk/debug/

# If you ever need a new Container, because it got into a wierd state, just call:
npm run android_arm64:devcontainer:fresh
```


## Example Workflow Using VS-Code: 

### Requirements: 
- All of the above 
- [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) Extension for VS-Code 
```bash 
# Do the same steps to create a container
npm install
#  build the Docker Image
npm run android_arm64:devcontainer:build
# Let's create a container
npm run android_arm64:devcontainer:up
```
In VS-Code now Open the Command Prompt: use `>Attach To Running Container`
find the Container using the `cuddlebuild:android_arm64` image and press enter. 
A new VS-Code window should open that lives inside the Container. ✨
therefore running a build like 
```
./scripts/android/cmake.sh 
```
Should just work / or give you the same error as in CI. 

![Screen_Recording_Of_Going_Into_VS_CODE](https://user-images.githubusercontent.com/9611612/233138266-4dd49973-0474-44c3-84fc-060dc19716b3.gif)


## Help Section 
### Q: Why is it slow?
Currently all Tasks are probably writing to the workspace-folder, which is mounted on the Host's OS. Unless you are running this on Linux, this is very costly. 

### Q: Get a Container Engine On MacOS: 
- Grab [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Mozillians: Get a licence in servicedesk!)
- Install With Recommended Settings
- If on M1
  - Go to Docker Settings -> "Features in development" 
  - Enable "Use Rosetta for x86/amd64 emulation"

![Screenshot Of docker setting](https://user-images.githubusercontent.com/9611612/233135351-563d42bb-8d5c-44c2-acf4-61d04a6354d0.png)
- If Docker was loaded correctly, in a new terminal `docker ps` should not error and instead show something similar to: 

```bash 
(base) basti@MBP-von-Basti ~ % docker ps
CONTAINER ID   IMAGE                       COMMAND                  CREATED       STATUS       PORTS     NAMES

```




### Q: (M1) - Vscode consumes all i/o to do sig-checks: 
If you feel fancy, you can disable them, or wait until it's done. 
```
"extensions.verifySignature": false
```

### How can i quickly run something in a container?
Step 1: 
Get the Container ID using `docker ps`
```
docker ps
CONTAINER ID   IMAGE                       COMMAND                  CREATED          STATUS          PORTS     NAMES
584389fe6cc2   cuddlebuild:android-arm64   "/bin/sh -c 'echo Co…"   26 minutes ago   Up 26 minutes             sweet_franklin
```

The Container ID of the container i want is `584389fe6cc2` - therefore you can now run for example:

```
docker exec -it 584389fe6cc2 watch ccache -s
```
or to get an interactive shell: 
```
docker exec -it 584389fe6cc2 bash
```