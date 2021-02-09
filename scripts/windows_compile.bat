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
    if "%1" NEQ "--prod" SET SHOW_HELP=T
  )
)

if "%SHOW_HELP%" == "T" (
  ECHO "Options:"
  ECHO "  -h|--help            Help menu"
  ECHO "  -p|--prod            Production build"
  EXIT 0
)

SET PROD_BUILD=F
if "%1"== "-p" SET PROD_BUILD=T
if "%1"== "--prod" SET PROD_BUILD=T

SET PROD_FLAGS=
if "%PROD_BUILD%" == "T" (
  ECHO Production build enabled
  SET PROD_FLAGS="CONFIG+=production"
) else (
  ECHO Staging build enabled
  SET PROD_FLAGS="CONFIG+=inspector"
)

ECHO Checking required commands...
CALL :CheckCommand python
CALL :CheckCommand nmake
CALL :CheckCommand cl
CALL :CheckCommand qmake

ECHO Importing languages...
python scripts\importLanguages.py

ECHO Creating the project...
qmake -tp vc src/src.pro CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release %PROD_FLAGS%

IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure the project
  EXIT 1
)

IF NOT EXIST MozillaVPN.vcxproj (
  echo The VC project doesn't exist. Why?
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
