@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.


if exist RECKON.G1A  del RECKON.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"I:\9860sdk\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"I:\9860sdk\Tools\MakeAddinHeader363.exe" "V:\sw\reckon"
if not exist RECKON.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

