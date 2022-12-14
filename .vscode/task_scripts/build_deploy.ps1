# Path where bin is compiled
$BIN_DIR = $PSScriptRoot+"/../bin"
$BUILD_DIR = $PSScriptRoot+"/../build"

# Path to the client/src/src.pro
$SRC_PATH = $PSScriptRoot+"../../../"

$FINAL_DIR = "C:\Program Files\Mozilla\Mozilla VPN"

# Enter a Visual Studio Command Prompt Session
. "$PSScriptRoot/vcvars.ps1"


$env:OPENSSL_ROOT_DIR="C:\MozillaVPNBuild\SSL"
$ENV:LIB="$ENV:LIB;$ENV:OPENSSL_ROOT_DIR\lib"
$env:LIBPATH="$ENV:LIBPATH;$ENV:OPENSSL_ROOT_DIR\lib"
# Stop Client / Service if alive
sc.exe stop MozillaVPNBroker
try
{
    Stop-Process -Name "MozillaVPN" -Force -ErrorAction SilentlyContinue
}
catch
{
  Write-Output "Mozilla VPN not running"
}
# From now on on Errors, we exit the script.
$ErrorActionPreference = "Stop"

# Create the OUT DIR, if not exists. 
If(!(test-path $BIN_DIR))
{
      New-Item -ItemType Directory -Force -Path $BIN_DIR
}
Set-Location $BIN_DIR


cmake --version
cmake -S $SRC_PATH -B $BUILD_DIR -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR
#cmake --build $BUILD_DIR --config RelWithDebInfo --target msi


if($LastExitCode -ne 0){
  Write-Output "BUILD Failed WITH exit:$LastExitCode "
  exit -1;
}

# Copy the compiled binaries to the OUT DIR
# Start the service / client. 
Copy-Item "$BUILD_DIR/src/Mozilla VPN.exe" -Destination $FINAL_DIR
sc.exe start MozillaVPNBroker

Set-Location $FINAL_DIR
#Start-Process "./Mozilla VPN.exe"