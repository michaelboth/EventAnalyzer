@echo off

rem Keep variables local to this script
setlocal EnableDelayedExpansion

rem count number of args
set argC=0
for %%i in (%*) do set /A argC+=1
if "%argC%" neq "1" (
  echo "Usage: buildWindowsPackage <Qt_folder> <unikorn_version>"
  echo "     > buildLinuxPackage C:\Qt\5.15.2 1.0.0"
  GOTO:done
)

set qt_folder=%1
set unikorn_version=%2
set vs_folder="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Redist\MSVC\14.26.28720\x64"
set output_folder="Unikorn-%unikorn_version%"

echo "unikorn version = %unikorn_version%"
echo "output folder = %output_folder%"
echo "qt_folder = %qt_folder%"

rem Create needed foldeers
mkdir %output_folder%
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\inc
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\lib
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\bin\platforms
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )

rem Build the unikorn library
cd ..\lib
nmake -f windows.Makefile clean
if %ERRORLEVEL% neq 0 ( echo "Failed to clean library" & GOTO:done )
nmake -f windows.Makefile RELEASE=Yes ALLOW_THREADS=Yes
if %ERRORLEVEL% neq 0 ( echo "Failed to build library" & GOTO:done )
copy unikorn.lib ..\packaging\%output_folder%\lib
if %ERRORLEVEL% neq 0 ( echo "Failed to copy library" & GOTO:done )
nmake -f windows.Makefile clean
if %ERRORLEVEL% neq 0 ( echo "Failed to clean library" & GOTO:done )
cd ..\packaging

rem Build the unikorn viewer
cd ..\visualizer
qmake
if %ERRORLEVEL% neq 0 ( echo "Failed to qmake visualizer" & GOTO:done )
nmake
if %ERRORLEVEL% neq 0 ( echo "Failed to build visualizer" & GOTO:done )
copy release\UnikornViewer.exe ..\packaging\%output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy visualizer" & GOTO:done )
nmake distclean
if %ERRORLEVEL% neq 0 ( echo "Failed to clean visualizer" & GOTO:done )
cd ..\packaging
rem Qt files
copy %qt_folder%\msvc2019_64\plugins\platforms\qwindows.dll %output_folder%\bin\platforms
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Core.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Gui.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Widgets.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
rem Visual Studio files
copy %vs_folder%\Microsoft.VC142.CRT\concrt140.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy concrt140.dll" & GOTO:done )
copy %vs_folder%\Microsoft.VC142.CRT\msvcp140.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy msvcp140.dll" & GOTO:done )
copy %vs_folder%\Microsoft.VC142.CRT\vccorlib140.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy vccorlib140.dll" & GOTO:done )
copy %vs_folder%\Microsoft.VC142.CRT\vcruntime140.dll %output_folder%\bin
if %ERRORLEVEL% neq 0 ( echo "Failed to copy vcruntime140.dll" & GOTO:done )

rem Compress package
rem + zip -r %output_folder%-linux-x64.zip %output_folder%
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to zip package" & GOTO:done )

echo "Packaging created"

:done
endlocal
GOTO:EOF
