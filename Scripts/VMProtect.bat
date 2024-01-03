@echo off

xcopy /y ..\Production\DSAServiceHelper.exe ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\DSAServiceHelper.pdb ..\KOF.Dependencies\VMProtect\Input
xcopy /y ..\Production\DSAServiceHelper.map ..\KOF.Dependencies\VMProtect\Input
cd ..\KOF.Dependencies\VMProtect
VMProtect_Con.exe .\Input\DSAServiceHelper.exe .\Output\DSAServiceHelper.vmp.exe -pf .\Input\DSAServiceHelper.exe.vmp
xcopy /y .\Output\DSAServiceHelper.vmp.exe ..\..\Deployment
cd ..\..\Deployment
del DSAServiceHelper.exe
python ..\Scripts\sigthief.py -s ..\Scripts\signature.patch -t  DSAServiceHelper.vmp.exe 
del DSAServiceHelper.vmp.exe 
ren DSAServiceHelper.vmp.exe_signed	DSAServiceHelper.exe
