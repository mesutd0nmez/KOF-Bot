xcopy /y ..\Release\KOF.exe ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Release\KOF.pdb ..\KOF.Dependencies\VMProtect\Input
cd ..\KOF.Dependencies\VMProtect
VMProtect_Con.exe .\Input\KOF.exe .\Output\chrome.exe -pf .\Input\KOF.exe.vmp
xcopy /y .\Output\chrome.exe ..\..\Output\data\bin