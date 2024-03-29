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
if ERRORLEVEL 1 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\visualizer
if ERRORLEVEL 1 ( echo "Failed to create folder" & GOTO:done )
mkdir %output_folder%\visualizer\windows
if ERRORLEVEL 1 ( echo "Failed to create folder" & GOTO:done )

rem Copy the relevant files
copy ..\LICENSE %output_folder%
copy ..\Unikorn_Introduction.pdf %output_folder%
copy ..\GETTING_STARTED.txt %output_folder%
xcopy /E/I ..\examples %output_folder%\examples
xcopy /E/I ..\src %output_folder%\src
xcopy /E/I ..\inc %output_folder%\inc

rem Build the unikorn viewer
cd ..\visualizer
qmake
if ERRORLEVEL 1 ( echo "Failed to qmake visualizer" & GOTO:done )
nmake
if ERRORLEVEL 1 ( echo "Failed to build visualizer" & GOTO:done )
copy release\UnikornViewer.exe ..\packaging\%output_folder%\visualizer\windows
if ERRORLEVEL 1 ( echo "Failed to copy visualizer" & GOTO:done )
nmake distclean
if ERRORLEVEL 1 ( echo "Failed to clean visualizer" & GOTO:done )
cd ..\packaging

rem Add support files for UnikornViewer.exe
%qt_folder%\msvc2019_64\bin\windeployqt %output_folder%\visualizer\windows

rem Compress package
powershell Compress-Archive %output_folder% %output_folder%-win64.zip
if ERRORLEVEL 1 ( echo "Failed to zip package" & GOTO:done )

echo "Unikorn Windows package created"

:done
endlocal
GOTO:EOF
