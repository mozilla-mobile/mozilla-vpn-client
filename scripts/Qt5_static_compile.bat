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

IF "%BUILDDIR%" == "" (
  SET BUILDDIR=C:\MozillaVPNBuild
)

ECHO Using Build Directory %BUILDDIR%

IF "%~1"=="" (
  CALL :Usage
  EXIT 1
)

IF NOT EXIST %1 (
  ECHO %1 doesn't exist.
  EXIT /B 1
)

IF "%~2"=="clean" (
  CALL :Clean %1
  EXIT /B 0
)
CALL :CheckRequirements

ECHO **********************************
ECHO Building Qt statically for Windows
ECHO **********************************

pushd %1

IF NOT EXIST configure.bat (
  ECHO This doesn't look like the QT5.15 source folder.
  EXIT /B 1
)

ECHO Configuring build...

CALL configure -static -opensource -debug-and-release -no-dbus -no-feature-qdbus -confirm-license -strip -silent -no-compile-examples -nomake tests -make libs -no-sql-psql -qt-sqlite -skip qt3d -skip webengine -skip qtmultimedia -skip qtserialport -skip qtsensors -skip qtgamepad -skip qtwebchannel -skip qtandroidextras -feature-imageformat_png -qt-libpng -qt-zlib -recheck-all -openssl-linked -I %BUILDDIR%\include -L %BUILDDIR%\lib -prefix %BUILDDIR% --verbose

rem This check is disabled, since configure returns non-zero due to QDoc build warnings.  It will fail later...
rem IF %ERRORLEVEL% NEQ 0 (
rem  ECHO Failed to configure QT5.
rem  EXIT /B 1
rem )

rem Let nmake use multiple cores
set CL=/MP

ECHO Compiling...
nmake
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile QT5.
  EXIT /B 1
)

ECHO Installing artifacts...
nmake install
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to install QT5.
  EXIT /B 1
)

popd
ECHO Qt build complete.
EXIT /B 0

:Clean
ECHO *********************
ECHO Cleaning Qt build...
ECHO *********************
pushd %1
nmake clean
popd
ECHO Clean complete.
EXIT /B 0

:Usage
ECHO %0 C:\Path\To\Qt\Source optional: clean
EXIT /B 0

:CheckRequirements
ECHO Checking required commands...
CALL :CheckCommand perl
CALL :CheckCommand nasm
CALL :CheckCommand python
CALL :CheckCommand nmake
CALL :CheckCommand cl
EXIT /B 0

:CheckCommand
WHERE %~1 > nul
IF %ERRORLEVEL% NEQ 0 (
  ECHO Command `%~1` has not been found.
  EXIT 1
)
ECHO Found %~1
EXIT /B 0