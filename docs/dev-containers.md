# Using Dev-Containers 

Dev-Containers are a small experiment, they allow you to run our CI docker Images locally. 

## Requirements: 
- Node
- A Container Engine (Docker, Podman, lima, Rancher)

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



## Help Section 
### Q: Why is it slow?
Currently all Tasks are probably writing to the workspace-folder, which is mounted on the Host's OS. Unless you are running this on Linux, this is very costly. 
