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

ECHO This script compiles Qt5 statically for windows

IF [%2] == [] (
  ECHO Usage: %1 /openssl/src/path /Qt5.15/src/path
  EXIT 1
)

IF NOT EXIST %1 (
  ECHO %1 doesn't exist.
  EXIT 1
)

IF NOT EXIST %2 (
  ECHO %2 doesn't exist.
  EXIT 1
)

ECHO Checking required commands...
CALL :CheckCommand perl
CALL :CheckCommand nasm
CALL :CheckCommand python
CALL :CheckCommand nmake
CALL :CheckCommand cl

ECHO Compiling openssl...
cd %1

perl Configure VC-WIN64A --release --prefix=c:\MozillaVPNBuild --openssldir=c:\MozillaVPNBuild\SSL
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure OpenSSL.
  EXIT 1
)

nmake
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile OpenSSL.
  EXIT 1
)

nmake install
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to install OpenSSL.
  EXIT 1
)

ECHO Compiling QT5...
cd %2
IF NOT EXIST configure.bat (
  ECHO This doesn't look like the QT5.15 source folder.
  EXIT /B 1
)

configure -static -opensource -release -no-dbus -no-feature-qdbus -confirm-license -strip -silent -no-compile-examples -nomake tests -make libs -no-sql-psql -no-sql-sqlite -skip qt3d -skip webengine -skip qtmultimedia -skip qtserialport -skip qtsensors -skip qtwebsockets -skip qtgamepad -skip qtwebchannel -skip qtandroidextras -feature-imageformat_png -qt-libpng -qt-zlib -recheck-all -openssl-linked -I c:\MozillaVPNBuild\include -L c:\MozillaVPNBuild\lib -prefix c:\MozillaVPNBuild
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure QT5.
  EXIT /B 1
)

nmake
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile QT5.
  EXIT /B 1
)

nmake install
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to install QT5.
  EXIT /B 1
)

ECHO All done.
EXIT 0

:CheckCommand
WHERE %~1 > nul
IF %ERRORLEVEL% NEQ 0 (
  ECHO Command `%~1` has not been found.
  EXIT 1
)
