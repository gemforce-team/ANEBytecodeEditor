call %AIRSDK%\bin\compc.bat +configname=air -source-path AS3/src -include-sources AS3/src -swf-version=11 -output ANEBytecodeEditor.swc -debug=false
call %AIRSDK%\bin\aasdoc.bat -doc-sources AS3/src -compiler.source-path AS3/src -swf-version=11 -output rawasdoc -debug=false -keep-xml=true -skip-xsl=true
copy /Y Native\BytecodeEditor\Release\BytecodeEditor.dll ANEBytecodeEditor.dll
move /Y ANEBytecodeEditor.swc ANEBytecodeEditor.zip
xcopy /I /Y rawasdoc\tempdita\* docs
del docs\ASDoc_Config.xml docs\overviews.xml
powershell -command "Expand-Archive -Force ANEBytecodeEditor.zip ."
rem The next line won't work because it uses backslashes in the ZIP path rather than forward slashes. Fuck Windows
rem powershell -command "Compress-Archive -Update -DestinationPath ANEBytecodeEditor.zip -Path docs"
move /Y ANEBytecodeEditor.zip ANEBytecodeEditor.swc
call %AIRSDK%\bin\adt.bat -package -target ane ANEBytecodeEditor.ane descriptor.xml -platform Windows-x86 library.swf ANEBytecodeEditor.dll -swc ANEBytecodeEditor.swc
del /Q ANEBytecodeEditor.zip library.swf catalog.xml
rmdir /S /Q rawasdoc