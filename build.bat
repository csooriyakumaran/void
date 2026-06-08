@echo off

set LIBS=^
    -luser32 ^
    -lgdi32 ^
    -lkernel32 ^
    -ladvapi32 ^
    -ldwmapi ^
    -lopengl32

set SRC=^
    .\src\main.c ^
    .\src\base.c ^
    .\src\vector-math.c ^
    .\src\renderer.c 


clang ^
    -std=c11 ^
    -Wall ^
    -g ^
    %SRC% ^
    -o .\bin\void.exe ^
    %LIBS%

