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
  if "%1" == "--help" SET SHOW_HELP=T

  if "%1" NEQ "-t" (
    if "%1" NEQ "--test" (
      if "%1" NEQ "-w" (
        if "%1" NEQ "--webextension" (
          if "%1" NEQ "--debug" (
             SET SHOW_HELP=T
          )
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
  ECHO "  --debug               Build a debug version"
  EXIT 0
)


IF "%BUILDDIR%" == "" (
   SET BUILDDIR=C:\MozillaVPNBuild
)
   ECHO Using Build Directory %BUILDDIR%



SET TEST_BUILD=F
if "%1"== "-t" SET TEST_BUILD=T
if "%1"== "--test" SET TEST_BUILD=T

SET WEBEXTENSION_BUILD=F
if "%1"== "-w" SET WEBEXTENSION_BUILD=T
if "%1"== "--webextension" SET WEBEXTENSION_BUILD=T

SET DEBUG_BUILD=F
if "%1"== "--debug" SET DEBUG_BUILD=T
if "%2"== "--debug" SET DEBUG_BUILD=T

SET BUILD_CONF=Release
if %DEBUG_BUILD% ==T (
  SET BUILD_CONF=Debug
)

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
CALL :CheckCommand git
CALL :CheckCommand python
CALL :CheckCommand cl
CALL :CheckCommand qmake

git submodule init
git submodule update --remote --depth 1 i18n

ECHO Copying the installer dependencies...
CALL :CopyDependency libcrypto-1_1-x64.dll %BUILDDIR%\SSL\bin\libcrypto-1_1-x64.dll
CALL :CopyDependency libssl-1_1-x64.dll %BUILDDIR%\SSL\bin\libssl-1_1-x64.dll

ECHO "Checking vctools in %VCToolsRedistDir%"
CALL :CopyDependency Microsoft_VC143_CRT_x86.msm "%VCToolsRedistDir%\\MergeModules\\Microsoft_VC143_CRT_x86.msm"
CALL :CopyDependency Microsoft_VC143_CRT_x64.msm "%VCToolsRedistDir%\\MergeModules\\Microsoft_VC143_CRT_x64.msm"

ECHO Importing languages...
python scripts\utils/import_languages.py

ECHO Generating glean samples...
python scripts\utils\generate_glean.py

ECHO BUILD_BUILD = %DEBUG_BUILD%

IF %DEBUG_BUILD%==T (
  ECHO Generating Debug Build for the extension bridge
  CD extension\bridge

  cargo build --debug
  IF %ERRORLEVEL% NEQ 0 (
    ECHO cargo failed for the extension!
    EXIT 1
  )

  CP target/debug/mozillavpnnp.exe ..\..
  CD ..\..
)

IF %DEBUG_BUILD%==F (
  ECHO Generating Release Build for the extension bridge
  CD extension\bridge

  cargo build --release
  IF %ERRORLEVEL% NEQ 0 (
    ECHO cargo failed for the extension!
    EXIT 1
  )

  CP target/release/mozillavpnnp.exe ..\..
  CD ..\..
)

ECHO Creating the project with flags: %FLAGS%

if %DEBUG_BUILD% == T (
  ECHO Generating Debug Project
  qmake src/src.pro CONFIG+=debug %FLAGS%
  xcopy /y debug\ release\
)
if %DEBUG_BUILD% == F (
  ECHO Generating Release Build
  qmake src/src.pro CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release CONFIG+=force_debug_info %FLAGS%
)

IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure the project
  EXIT 1
)


ECHO Compiling the balrog.dll...
CALL balrog\build.cmd

IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile balrog.dll
  EXIT 1
)
CALL :CopyDependency balrog.dll balrog\x64\balrog.dll

ECHO Compiling the tunnel.dll...
CALL windows\tunnel\build.cmd
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile tunnel.dll.
  EXIT 1
)

ECHO Fetching Split-Tunnel Driver...
CALL PowerShell -NoProfile -ExecutionPolicy Bypass -Command "& './windows/split-tunnel/get.ps1'"
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to fetch the Split-Tunnel Driver
  EXIT 1
)


nmake 
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to build the project
  EXIT 1
)
ECHO Build mozillavpn.exe

if %DEBUG_BUILD% == T (
  REM We need to move the exes in debug so the installer can find them
  xcopy /y debug\*.exe .\
)

ECHO Creating the installer...
CALL windows\installer\build.cmd
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to create the installer.
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
