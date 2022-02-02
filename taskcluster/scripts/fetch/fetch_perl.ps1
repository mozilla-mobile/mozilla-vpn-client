

$PERL_URL = "https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit.zip"
$BUILD_TOOLS_DIR = "C:\MozillaVPNBuild"
if(!(Test-Path $BUILD_TOOLS_DIR)){
    New-Item -Path $BUILD_TOOLS_DIR -ItemType "directory"
    New-Item -Path $BUILD_TOOLS_DIR\.tmp -ItemType "directory"
}

if(!(Test-Path  $BUILD_TOOLS_DIR\perl\)){
    Invoke-WebRequest -Uri $PERL_URL -OutFile $BUILD_TOOLS_DIR\perl.zip
    Expand-Archive -Path $BUILD_TOOLS_DIR\perl.zip -DestinationPath $BUILD_TOOLS_DIR/perl
}

echo "Installed Perl in $BUILD_TOOLS_DIR/perl"