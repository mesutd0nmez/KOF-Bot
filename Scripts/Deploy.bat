@echo off

xcopy /y ..\Production\DSAServiceHelper.exe ..\Tools\VMProtect\Input
xcopy /y ..\Production\DSAServiceHelper.pdb ..\Tools\VMProtect\Input
xcopy /y ..\Production\DSAServiceHelper.map ..\Tools\VMProtect\Input

cd ..\Tools\VMProtect
VMProtect_Con.exe .\Input\DSAServiceHelper.exe .\Output\DSAServiceHelper.vmp.exe -pf .\Input\DSAServiceHelper.exe.vmp
xcopy /y .\Output\DSAServiceHelper.vmp.exe ..\..\Production

cd ..\..\Production
del DSAServiceHelper.exe
ren DSAServiceHelper.vmp.exe DSAServiceHelper.exe

del Update.zip
powershell Compress-Archive -Path DSAServiceHelper.exe -DestinationPath Update.zip -Force