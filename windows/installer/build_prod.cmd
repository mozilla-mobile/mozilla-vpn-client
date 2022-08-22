@echo off
rem SPDX-License-Identifier: MIT
rem Copyright (C) 2019 WireGuard LLC. All Rights Reserved.
rem Copyright (C) 2019 Edge Security LLC. All Rights Reserved.

setlocal
set PATHEXT=.exe
set BUILDDIR=%~dp0
set ARTIFACTDIR=%TASK_WORKDIR%\artifacts
set FETCHESDIR=%TASK_WORKDIR%\fetches
cd /d %BUILDDIR% || exit /b 1

set WIX_CANDLE_FLAGS=-nologo -ext WiXUtilExtension
set WIX_LIGHT_FLAGS=-nologo -spdb -ext WixUtilExtension

if exist .deps\prepared goto :build
:installdeps
	rmdir /s /q .deps 2> NUL
	mkdir .deps || goto :error
	cd .deps || goto :error
	call :download wix-binaries.zip https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311-binaries.zip 2c1888d5d1dba377fc7fa14444cf556963747ff9a0a289a3599cf09da03b9e2e || goto :error
	echo [+] Extracting wix-binaries.zip
	mkdir wix\bin || goto :error
	powershell -command "Expand-Archive" -Path wix-binaries.zip -DestinationPath wix\bin || exit /b 1
	echo [+] Cleaning up wix-binaries.zip
	del wix-binaries.zip || goto :error
	copy /y NUL prepared > NUL || goto :error
	cd .. || goto :error

:build
	echo [+] Extracting %FETCHESDIR%\unsigned.zip
	powershell -command "Expand-Archive" -Path %FETCHESDIR%\unsigned.zip -DestinationPath %TASK_WORKDIR%\build\src || exit /b 1
	set WIX=%BUILDDIR%.deps\wix\
	call :msi x64 || goto :error

:success
	echo [+] Success.
	exit /b 0

:download
	echo [+] Downloading %1
	powershell -command "Invoke-WebRequest" -Uri %2 -OutFile %1 || exit /b 1
	echo [+] Verifying %1
	for /f %%a in ('CertUtil -hashfile %1 SHA256 ^| findstr /r "^[0-9a-f]*$"') do if not "%%a"=="%~3" exit /b 1
	goto :eof

:msi
	if not exist "%ARTIFACTDIR%" mkdir "%ARTIFACTDIR%"
	echo [+] Compiling %1
       "%WIX%bin\candle" %WIX_CANDLE_FLAGS% -dPlatform=%1 -out "%ARTIFACTDIR%\MozillaVPN.wixobj" -arch %1 MozillaVPN_prod.wxs || exit /b %errorlevel%
	echo [+] Linking %1
       "%WIX%bin\light" %WIX_LIGHT_FLAGS% -out "%ARTIFACTDIR%/MozillaVPN.msi" "%ARTIFACTDIR%\MozillaVPN.wixobj" || exit /b %errorlevel%
	goto :eof

:error
	echo [-] Failed with error #%errorlevel%.
	cmd /c exit %errorlevel%
