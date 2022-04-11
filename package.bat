call %AIRSDK%\bin\compc.bat +configname=air -source-path AS3/src -include-sources AS3/src -swf-version=11 -output ANEBytecodeEditor.swc -debug=true
copy /Y ANEBytecodeEditor.swc ANEBytecodeEditor.zip
powershell -command "Expand-Archive -Force ANEBytecodeEditor.zip ."
call %AIRSDK%\bin\adt.bat -package -target ane ANEBytecodeEditor.ane descriptor.xml -platform Windows-x86 library.swf ANEBytecodeEditor.dll -swc ANEBytecodeEditor.swc
del ANEBytecodeEditor.zip
del library.swf
del catalog.xml