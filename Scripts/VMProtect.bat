xcopy /y ..\Production\Discord.exe ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Discord.pdb ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Updater2.exe ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Updater2.pdb ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Connector.dll ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Connector.pdb ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Adapter.dll ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\Adapter.pdb ..\KOF.Dependencies\VMProtect\Input
cd ..\KOF.Dependencies\VMProtect
VMProtect_Con.exe .\Input\Discord.exe .\Output\Discord.vmp.exe -pf .\Input\Discord.exe.vmp
VMProtect_Con.exe .\Input\Updater2.exe .\Output\Updater2.vmp.exe -pf .\Input\Updater2.exe.vmp
VMProtect_Con.exe .\Input\Connector.dll .\Output\Connector.vmp.dll -pf .\Input\Connector.dll.vmp
VMProtect_Con.exe .\Input\Adapter.dll .\Output\Adapter.vmp.dll -pf .\Input\Adapter.dll.vmp
xcopy /y .\Output\Discord.vmp.exe ..\..\Deployment
xcopy /y .\Output\Updater2.vmp.exe ..\..\Deployment
xcopy /y .\Output\Connector.vmp.dll ..\..\Deployment
xcopy /y .\Output\Adapter.vmp.dll ..\..\Deployment