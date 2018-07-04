@echo off

IF NOT EXIST bin mkdir bin
pushd bin

set BinaryName=highp
set ExeName=%BinaryName%.exe

set CompilerFlags= -nologo -I..\src\ ^
 -GR- -Gm- -EHa- -Oi -fp:fast -fp:except- -GS- -Gs0x100000 ^
-D_CRT_SECURE_NO_WARNINGS=1 -DNOMINMAX=1 ^
-W4 -WX -we4061 -wd4100 -wd4189 -wd4127 -wd4505 -wd4201 ^
-wd4533 -wd4389 -wd4244 -wd4820 

set LinkFlags= /INCREMENTAL:NO /MACHINE:X64 /NODEFAULTLIB /STACK:0x100000,0x100000 /SUBSYSTEM:CONSOLE /WX kernel32.lib user32.lib Ws2_32.lib  

REM kernel32.lib user32.lib gdi32.lib shell32.lib Shlwapi.lib
REM libucrt.lib libvcruntime.lib libcmt.lib 

set opt1=%1

IF "%opt1%"=="" (
    goto DEBUG_FLAGS
)
IF "%opt1%"=="release" (
    goto RELEASE_FLAGS
)


:DEBUG_FLAGS
set CompilerFlags=%CompilerFlags% -Zo -Zi -Od -DDEBUG=1
goto START_COMPILE

:RELEASE_FLAGS
set CompilerFlags=%CompilerFlags%  -Zo -Zi -O2
set LinkFlags=%LinkFlags% /OPT:REF,ICF=1

:START_COMPILE

del %BinaryName%.pdb

set StartTime=%time%

echo %StartTime%
echo.
echo ... BUILDING %ExeName% ...

IF "%VCINSTALLDIR%"=="" (
    call vcvarsall x64 > nul
)

@echo on
cl %CompilerFlags% ..\src\win32_msvc.c ..\src\win32_twitch_client.cpp ^
/Fe%ExeName% /link %LinkFlags%

@echo off
popd



