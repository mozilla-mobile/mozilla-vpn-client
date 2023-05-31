# Pre-requisites

## Get Qt

Get a static build of Qt made built in our CI.

https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-win.latest/artifacts/public%2Fbuild%2Fqt6_win.zip

Unzip the folder and remember the location for the configure step.

## Conda

Have [miniconda installed](https://repo.anaconda.com/miniconda/Miniconda3-py310_23.1.0-1-Windows-x86_64.exe).

Open Powershell and run Conda

    conda init

You may need to enable [powershell scripts.](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.security/set-executionpolicy?view=powershell-7.3). You can call ```Set-ExecutionPolicy -ExecutionPolicy Unrestricted ```

For the setup you need to use powershell.

```powershell
$ conda env create -f env.yml
$ conda activate vpn
# Setup the conda env for clang-cl compilation
$ ./scripts/windows/conda_setup_win_sdk.ps1
# Setup the conda env to use a prebuild Qt from Moz-CI
$ ./scripts/windows/conda_setup_win_qt.ps1
# Install conda Packages only needed on Windows
$ ./scripts/windows/conda_install_extras.ps1
# Reactivate the env to apply the changes:
$ conda deactivate
$ conda activate vpn
```
# Build

Make the build directory

    mkdir build

Configure

    cmake -S . -B build -GNinja

Compile

    cmake --build build
