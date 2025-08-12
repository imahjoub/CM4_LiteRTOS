echo off

set TYP_MCU=%2

set BUILD_COMMAND=%CD%\..\tools\util\bin\make.exe -f %CD%\..\Make\make_000.gmk all TYP_OS=win

echo Build command is: %BUILD_COMMAND%

%BUILD_COMMAND%
