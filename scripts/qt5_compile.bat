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

IF [%2] == [] (
  ECHO Usage: %1 /openssl/src/path /Qt5.15/src/path
  EXIT /B 1
)

IF NOT EXIST %1 (
  ECHO openssl path doesn't exist at %1.
  EXIT /B 1
)

IF NOT EXIST %2 (
  ECHO Qt source path does not exists at %2.
  EXIT /B 1
)

IF "%~3"=="clean" (
  CALL :Clean %0 %1 %2
)

ECHO This script compiles openssl and Qt statically for Windows.

CALL %0\..\openssl_compile %1
CALL :CheckError "Openssl failed to compile"
CALL %0\..\Qt5_static_compile %2
CALL :CheckError "Qt failed to compile."
EXIT /B 0

:CheckError
IF %ERRORLEVEL% NEQ 0 (
  ECHO %1.
  EXIT 1
)
EXIT /B 0

:Clean
ECHO Cleaning openssl...
CALL %1\..\openssl_compile %2 clean
ECHO Cleaning Qt...
CALL %1\..\Qt5_static_compile %3 clean
EXIT 0

:Usage
  ECHO Usage: %1 C:\Openssl\Source\Path C:\Qt\Source\Path optional: clean
EXIT /B 1