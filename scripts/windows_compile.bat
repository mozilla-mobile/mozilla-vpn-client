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
  if "%1" NEQ "-p" (
    if "%1" NEQ "--prod" (
      if "%1" NEQ "-t" (
        if "%1" NEQ "--test" (
          SET SHOW_HELP=T
        )
      )
    )
  )
)

if "%SHOW_HELP%" == "T" (
  ECHO "Options:"
  ECHO "  -h|--help            Help menu"
  ECHO "  -p|--prod            Production build"
  ECHO "  -t|--test            Test mode"
  EXIT 0
)

SET PROD_BUILD=F
if "%1"== "-p" SET PROD_BUILD=T
if "%1"== "--prod" SET PROD_BUILD=T

SET TEST_BUILD=F
if "%1"== "-t" SET TEST_BUILD=T
if "%1"== "--test" SET TEST_BUILD=T

SET FLAGS=BUILD_ID=%VERSION%

if "%PROD_BUILD%" == "T" (
  ECHO Production build enabled
  SET FLAGS=%FLAGS% CONFIG+=production
) else (
  ECHO Staging build enabled
  SET FLAGS=%FLAGS% CONFIG+=inspector
)

if "%TEST_BUILD%" == "T" (
  ECHO Test build enabled
  SET FLAGS=%FLAGS% CONFIG+=DUMMY
) else (
  SET FLAGS=%FLAGS% CONFIG+=balrog
)

ECHO Checking required commands...
CALL :CheckCommand python
CALL :CheckCommand nmake
CALL :CheckCommand cl
CALL :CheckCommand qmake

ECHO Copying the installer dependencies...
CALL :CopyDependency libcrypto-1_1-x64.dll c:\MozillaVPNBuild\bin\libcrypto-1_1-x64.dll
CALL :CopyDependency libssl-1_1-x64.dll c:\MozillaVPNBuild\bin\libssl-1_1-x64.dll
CALL :CopyDependency libEGL.dll c:\MozillaVPNBuild\bin\libEGL.dll
CALL :CopyDependency libGLESv2.dll c:\MozillaVPNBuild\bin\libGLESv2.dll
CALL :CopyDependency Microsoft_VC142_CRT_x86.msm "c:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC\\14.28.29325\\MergeModules\\Microsoft_VC142_CRT_x86.msm"
CALL :CopyDependency Microsoft_VC142_CRT_x64.msm "c:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC\\14.28.29325\\MergeModules\\Microsoft_VC142_CRT_x64.msm"

ECHO Importing languages...
python scripts\importLanguages.py

ECHO Extract version...
FOR /F "tokens=2* delims==" %%A IN ('FINDSTR /IC:":VERSION" version.pri') DO call :SetVersion %%A

ECHO Creating the project...
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

ECHO Compiling the tunnel.dll...
CALL windows\tunnel\build.cmd
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
  set VERSION=%VERSION%.%date:~-4%%date:~4,2%%date:~7,2%%time:~0,2%%time:~3,2%
  goto :EOF
