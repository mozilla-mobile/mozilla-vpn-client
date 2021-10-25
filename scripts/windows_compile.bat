:: This Source Code Form is subject to the terms of the Mozilla Public
:: License, v. 2.0. If a copy of the MPL was not distributed with this
:: file, You can obtain one at http://mozilla.org/MPL/2.0/.

@ECHO off
SETLOCAL

IF EXIST env.bat (
  CALL env.bat
)

IF "%selfWrapped%" == "" (
  :: This is necessary so that we can use "EXIT" to terminate the batch file,
  :: and all subroutines, but not the original cmd.exe
  SET selfWrapped=true
  %ComSpec% /s /c ""%~0" %*"
  GOTO :EOF
)

ECHO This script MozillaVPN for windows

IF NOT EXIST src (
  ECHO THis doesn't seem to be the root of the MozillaVPN repository.
  EXIT 1
)

SET SHOW_HELP=F

if "%1" NEQ "" (
  if "%1" == "-h" SET SHOW_HELP=T
  if "%1" == "-help" SET SHOW_HELP=T
  if "%1" NEQ "-t" (
    if "%1" NEQ "--test" (
      if "%1" NEQ "-w" (
        if "%1" NEQ "--webextension" (
          SET SHOW_HELP=T
        )
      )
    )
  )
)

if "%SHOW_HELP%" == "T" (
  ECHO "Options:"
  ECHO "  -h|--help            Help menu"
  ECHO "  -t|--test            Test mode"
  ECHO "  -w|--webextension    Enable the webExtension support"
  EXIT 0
)

SET TEST_BUILD=F
if "%1"== "-t" SET TEST_BUILD=T
if "%1"== "--test" SET TEST_BUILD=T

SET WEBEXTENSION_BUILD=F
if "%1"== "-w" SET WEBEXTENSION_BUILD=T
if "%1"== "--webextension" SET WEBEXTENSION_BUILD=T

ECHO Extract version...
FOR /F "tokens=2* delims==" %%A IN ('FINDSTR /IC:":VERSION" version.pri') DO call :SetVersion %%A

SET FLAGS=BUILD_ID=%VERSION%

if "%TEST_BUILD%" == "T" (
  ECHO Test build enabled
  SET FLAGS=%FLAGS% CONFIG+=DUMMY
) else (
  SET FLAGS=%FLAGS% CONFIG+=balrog
)

if "%WEBEXTENSION_BUILD%" == "T" (
  ECHO Web-Extension support enabled
  SET FLAGS=%FLAGS% CONFIG+=webextension
)

ECHO Checking required commands...
CALL :CheckCommand python
CALL :CheckCommand nmake
CALL :CheckCommand cl
CALL :CheckCommand qmake


ECHO Importing languages...
git submodule update --remote --depth 1 i18n
python scripts\importLanguages.py

ECHO Generating glean samples...
python scripts\generate_glean.py

ECHO Build Inspector
CALL npm --prefix ./inspector run build

qmake -tp vc extension\app\app.pro CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure the project
  EXIT 1
)

IF NOT EXIST mozillavpnnp.vcxproj (
  echo The VC project doesn't exist. Why?
  EXIT 1
)

ECHO Cleaning up the project...
MSBuild -t:Clean -p:Configuration=Release mozillavpnnp.vcxproj
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)

MSBuild -t:Build -p:Configuration=Release mozillavpnnp.vcxproj
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to build the project
  EXIT 1
)

ECHO Creating the project with flags: %FLAGS%
qmake -tp vc src/src.pro CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release %FLAGS%

IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure the project
  EXIT 1
)

IF NOT EXIST MozillaVPN.vcxproj (
  echo The VC project doesn't exist. Why?
  EXIT 1
)

ECHO Compiling the balrog.dll...
CALL balrog\build.cmd
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)
CALL :CopyDependency balrog.dll balrog\x64\balrog.dll

ECHO Compiling the tunnel.dll...
CALL windows\tunnel\build.cmd
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)

ECHO Fetching Split-Tunnel Driver...
CALL PowerShell -NoProfile -ExecutionPolicy Bypass -Command "& './windows/split-tunnel/get.ps1'"
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)

ECHO Cleaning up the project...
MSBuild -t:Clean -p:Configuration=Release MozillaVPN.vcxproj
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)

MSBuild -t:Build -p:Configuration=Release MozillaVPN.vcxproj
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to build the project
  EXIT 1
)

ECHO Creating the installer...
CALL windows\installer\build.cmd
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to clean up the project
  EXIT 1
)

ECHO All done.
EXIT 0

:CheckCommand
  WHERE %~1 > nul
  IF %ERRORLEVEL% NEQ 0 (
    ECHO Command `%~1` has not been found.
    EXIT 1
  )
  goto :eof

:CopyDependency
  IF NOT EXIST %~1 (
    COPY /y "%~2" "%~1" > nul
    IF %ERRORLEVEL% NEQ 0 (
      ECHO Failed to copy the dependency `%~1`.
      EXIT 1
    )
  )
  goto :eof

:SetVersion
  for /f "tokens=1* delims=." %%A IN ("%1") DO call :ComposeVersion %%A
  goto :EOF

:ComposeVersion
  SET VERSION=%1
  SET T=%TIME: =0%
  SET VERSION=%VERSION%.%date:~-4%%date:~4,2%%date:~7,2%%T:~0,2%%T:~3,2%
  goto :EOF
