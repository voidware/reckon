@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.


if exist L3DGT.G1A  del L3DGT.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"C:\9860gsdk\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"C:\9860gsdk\Tools\MakeAddinHeader363.exe" "W:\hugh\9860g\reckon\l3d\l3dgt"
if not exist L3DGT.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

