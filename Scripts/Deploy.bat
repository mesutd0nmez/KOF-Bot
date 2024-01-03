@echo off

xcopy /y ..\Production\Adapter.dll ..\Deployment
xcopy /y ..\Production\Updater.exe ..\Deployment
xcopy /y ..\Production\KOF.Checksum.exe ..\Deployment