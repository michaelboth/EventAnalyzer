@echo off

rem Keep variables local to this script
setlocal EnableDelayedExpansion

rem count number of args
set argC=0
for %%i in (%*) do set /A argC+=1
if "%argC%" neq "2" (
  echo "Usage: buildWindowsPackage <Qt_folder> <unikorn_version>"
  echo "     > buildWindowsPackage C:\Qt\5.15.2 1.0.0"
  GOTO:done
)

set qt_folder=%1
set unikorn_version=%2
set vs_folder="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Redist\MSVC\14.26.28720\x64"
set output_folder="Unikorn-%unikorn_version%"

echo "unikorn version = %unikorn_version%"
echo "output folder = %output_folder%"
echo "qt_folder = %qt_folder%"
echo "vs_folder = %vs_folder%"

rem Create needed foldeers
mkdir %output_folder%
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\visualizer
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\visualizer\platforms
if %ERRORLEVEL% neq 0 ( echo "Failed to create folder" & GOTO:done )

rem Copy the relevant files
copy ..\README.md %output_folder%
copy ..\LICENSE %output_folder%
xcopy /E/I ..\examples %output_folder%\examples
xcopy /E/I ..\src %output_folder%\src
xcopy /E/I ..\inc %output_folder%\inc

rem Build the unikorn viewer
cd ..\visualizer
qmake
if %ERRORLEVEL% neq 0 ( echo "Failed to qmake visualizer" & GOTO:done )
nmake
if %ERRORLEVEL% neq 0 ( echo "Failed to build visualizer" & GOTO:done )
copy release\UnikornViewer.exe ..\packaging\%output_folder%\visualizer
if %ERRORLEVEL% neq 0 ( echo "Failed to copy visualizer" & GOTO:done )
nmake distclean
if %ERRORLEVEL% neq 0 ( echo "Failed to clean visualizer" & GOTO:done )
cd ..\packaging
rem Qt files
copy %qt_folder%\msvc2019_64\plugins\platforms\qwindows.dll %output_folder%\visualizer\platforms
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Core.dll %output_folder%\visualizer
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Gui.dll %output_folder%\visualizer
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
copy %qt_folder%\msvc2019_64\bin\Qt5Widgets.dll %output_folder%\visualizer
if %ERRORLEVEL% neq 0 ( echo "Failed to copy Qt file" & GOTO:done )
rem Visual Studio files
rem + copy %vs_folder%\Microsoft.VC142.CRT\concrt140.dll %output_folder%\visualizer
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to copy concrt140.dll" & GOTO:done )
rem + copy %vs_folder%\Microsoft.VC142.CRT\msvcp140.dll %output_folder%\visualizer
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to copy msvcp140.dll" & GOTO:done )
rem + copy %vs_folder%\Microsoft.VC142.CRT\vccorlib140.dll %output_folder%\visualizer
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to copy vccorlib140.dll" & GOTO:done )
rem + copy %vs_folder%\Microsoft.VC142.CRT\vcruntime140.dll %output_folder%\visualizer
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to copy vcruntime140.dll" & GOTO:done )

rem Compress package
rem + zip -r %output_folder%-linux-x64.zip %output_folder%
rem + if %ERRORLEVEL% neq 0 ( echo "Failed to zip package" & GOTO:done )

echo "Packaging created"

:done
endlocal
GOTO:EOF
