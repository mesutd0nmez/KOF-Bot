@echo off

del Update.zip
powershell Compress-Archive -Path ..\Deployment\DSAServiceHelper.exe -DestinationPath ..\Deployment\Update.zip -Force
echo Created Update.zip