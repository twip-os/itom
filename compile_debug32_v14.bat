TITLE compile debug x86 - %~dp0
CALL "%VS140COMNTOOLS%\vsvars32.bat"

msbuild.exe "%~dp0\ALL_BUILD.vcxproj" /p:configuration=debug /p:platform=win32

pause

REM for rebuild add "/t:rebuild" after /p:platform=win32