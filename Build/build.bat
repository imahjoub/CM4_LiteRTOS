echo off

set BUILD_COMMAND=%CD%\..\tools\util\bin\make.exe -f %CD%\..\Make\make_stm32f446re.gmk all TYP_OS=win

echo Build command is: %BUILD_COMMAND%

%BUILD_COMMAND%
