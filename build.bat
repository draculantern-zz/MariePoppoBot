@echo off

IF NOT EXIST bin mkdir bin
pushd bin

set ExeName=highp.exe

set DebugFlags= -Zo -Zi -Od
set ReleaseFlags= -O2

set CompilerFlags= -nologo -I..\src\ ^
-D_CRT_SECURE_NO_WARNINGS=1 -DNOMINMAX=1 -DDEBUG=1 ^
-W4 -WX -we4062 -we4061 -wd4100 -wd4189 -wd4127 -wd4505 -wd4201 -wd4533 -wd4389 ^
-wd4244 -GR- -Gm- -EHa- -Oi -fp:fast -fp:except- -GS-

set CompilerFlags=%CompilerFlags% %DebugFlags%

set LinkFlags=  /NOLOGO /INCREMENTAL:NO /MACHINE:X64 /NODEFAULTLIB /STACK:0xA00000,0xA00000 /SUBSYSTEM:CONSOLE /WX kernel32.lib user32.lib Ws2_32.lib  

REM kernel32.lib user32.lib gdi32.lib shell32.lib Shlwapi.lib
REM libucrt.lib libvcruntime.lib libcmt.lib 


set StartTime=%time%

echo %StartTime%
echo.
echo ... BUILDING %ExeName% ...

call vcvarsall x64 > nul
@echo on
cl %CompilerFlags% ..\src\win32_msvc.c ..\src\win32_twitch_client.cpp ^
/Fe%ExeName% /link %LinkFlags%

@echo off
popd



