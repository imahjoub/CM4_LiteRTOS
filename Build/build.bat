::----------------------------------------------------------------------------
:: @brief Build Script for Windows
::
:: @desc Sets the target MCU type, constructs the build command, and
::       invokes the Makefile to build the project.
::
:: @param %1    Typ BUILD (not used)
:: @param %2    Target MCU type
:: @return      Executes the build process
::----------------------------------------------------------------------------

echo off

set TYP_BUILD=%1
set TYP_MCU=%2

set BUILD_COMMAND=%CD%\..\tools\util\bin\make.exe -f %CD%\..\Make\make_000.gmk all TYP_OS=win

echo Build command is: %BUILD_COMMAND%

%BUILD_COMMAND%
