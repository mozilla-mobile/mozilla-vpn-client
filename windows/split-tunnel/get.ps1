# Fetches only the split-tunnel driver from https://github.com/mullvad/mullvadvpn-app-binaries
# To update the Driver, update the commit SHA :)
$COMMIT_SHA = "375cfc52255ec3beb15cc0713988ea995cb7582e"

$BASE_URI = "https://github.com/mullvad/mullvadvpn-app-binaries/raw/"+$COMMIT_SHA+"/x86_64-pc-windows-msvc/split-tunnel/win10/"
$STATUSFILE = $PSScriptRoot+"\.status"
$FILES = "mullvad-split-tunnel.cat","mullvad-split-tunnel.inf","mullvad-split-tunnel.sys","WdfCoinstaller01011.dll"

try{
    $statusContent = Get-Content $STATUSFILE -Encoding ASCII
    if( !($statusContent -eq $COMMIT_SHA)){
        "Wrong Commit, Pulling Driver"
        throw
    }
    foreach ($file in $FILES) {
        "Checking: $file"
        $path = ".\windows\split-tunnel\"+$file
        if(!(Test-Path $path)){
            "Missing $file"
            throw
        }

      }
    "Driver Files OK"
    exit
}
catch{
    "Cannot Reuse Old Files, fetching Driver"
}

if(!(Test-Path $STATUSFILE)){
    Remove-Item $STATUSFILE
}

foreach ($file in $FILES) {
    "Getting $file" 
    if((Test-Path $file)){
        Remove-Item $file
    }
    $download_uri = $BASE_URI + $file
    $targetFile = $PSScriptRoot+ "\"+$file
    Invoke-WebRequest -Uri $download_uri -OutFile $targetFile

}

Out-File -FilePath $STATUSFILE  -InputObject $COMMIT_SHA -Encoding ASCII