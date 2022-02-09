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

IF "%~1"=="" (
  CALL :Usage
  EXIT /B 1
)

IF "%~2"=="clean" (
  CALL :Clean %1
  EXIT /B 1
)

CALL :CheckRequirements


IF "%BUILDDIR%" == "" (
  SET BUILDDIR=C:\MozillaVPNBuild
)

ECHO Using Build Directory %BUILDDIR%

ECHO Compiling openssl...

pushd %1

perl Configure VC-WIN64A --release --prefix=%BUILDDIR% --openssldir=%BUILDDIR% no-tests -DOPENSSL_NO_ENGINE
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to configure OpenSSL.
  EXIT /B 1
)

set CL=/MP

nmake build_libs
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to compile OpenSSL.
  EXIT /B 1
)

nmake install_sw
IF %ERRORLEVEL% NEQ 0 (
  ECHO Failed to install OpenSSL.
  EXIT /B 1
)

popd

ECHO Build Complete.
EXIT /B 0

:Clean
pushd %1
ECHO *************************
ECHO Cleaning openssl build.
ECHO *************************
nmake clean
popd
ECHO Clean complete.
EXIT /B 0

:Usage
ECHO You must specify the path to openssl.
ECHO Usage Example: %0 C:\Your\Openssl\Path Optional: clean
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