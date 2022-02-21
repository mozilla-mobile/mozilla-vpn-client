
$REPO_ROOT_PATH =resolve-path "$PSScriptRoot../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH../../fetches"


. "$FETCHES_PATH/VisualStudio/enter_dev_shell.ps1"

python3 -m pip install -r requirements.txt
git submodule update --init --force --recursive --depth=1

#TODO: Add qt to path
$QTPATH = resolve-path ./fetches/QT_OUT/bin/
env:PATH ="$QTPATH;$env:PATH"


./scripts/windows_compile.bat -w