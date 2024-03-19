# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
git submodule update --init --recursive

$REPO_ROOT_PATH = resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR = resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH = resolve-path "$TASK_WORKDIR/fetches"
$SOURCE_DIR = resolve-path "."

$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/"

# Prep Env:
# Switch to the work dir, configure qt
Set-Location -Path $TASK_WORKDIR
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"

## Setup the conda environment
. $SOURCE_DIR/scripts/utils/call_bat.ps1  $FETCHES_PATH/Scripts/activate.bat
conda-unpack

# Conda Pack excpets to be run under cmd. therefore it will
# (unlike conda) ignore activate.d powershell scripts.
# So let's manually run the activation scripts.
#
$CONDA_PREFIX = $env:CONDA_PREFIX
$env:PATH="$CONDA_PREFIX\bin;$env:Path"
# 
$ACTIVATION_SCRIPTS = Get-ChildItem -Path "$CONDA_PREFIX\etc\conda\activate.d" -Filter "*.ps1"

foreach ($script in  $ACTIVATION_SCRIPTS)  {
    Write-Output "Activating: $CONDA_PREFIX\etc\conda\activate.d\$script"
    . "$CONDA_PREFIX\etc\conda\activate.d\$script"
}

mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"

cmake -S $SOURCE_DIR -B $BUILD_DIR -GNinja `
        -DCMAKE_BUILD_TYPE=Release `
        -DPYTHON_EXECUTABLE="$CONDA_PREFIX\python.exe" `
        -DGOLANG_BUILD_TOOL="$CONDA_PREFIX\bin\go.exe" `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
        -DBUILD_TESTS=OFF `
        -DWINTUN_FOLDER="not-existing" `
        -DCMAKE_PREFIX_PATH="$QTPATH/lib/cmake" `

cmake --build $BUILD_DIR --target translations
Write-Output "GENERATED FOLDER"
ls $BUILD_DIR/translations/generated
Write-Output "GENERATED FOLDER"

cat $BUILD_DIR/compile_commands.json
cmake --build $BUILD_DIR --target clang_tidy_report

New-Item -ItemType Directory -Path "$TASK_WORKDIR/artifacts" -Force
Copy-Item -Path $BUILD_DIR/clang-tidy/* -Destination $TASK_WORKDIR/artifacts
