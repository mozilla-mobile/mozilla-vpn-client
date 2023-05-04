
# conda_preset_generator

Can generate a UserPresets.json File for your current Conda Env, to use your conda-env in any cmake-preset compatible IDE. 

Tested: VS-Studio-Code, VSstudio2022, CLion

1. activate your chosen conda env `conda activate MY_Enviroment_To_export`
2. export it `npm run conda_preset_generator --name <name_for_cmake_preset>`

Now you can use it in cmake directly: 
`
cmake build --preset <name_for_cmake_preset>`

### VS-Code usage: 
Install [CMake-Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) - it should auto detect your preset. If not
Do `CTRL-P` -> `> CMake: Select Build Preset` and choose your preset. 
