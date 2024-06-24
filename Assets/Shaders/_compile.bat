@echo off
SETLOCAL

set "_glslc=C:\VulkanSDK\1.3.275.0\Bin\glslc.exe --target-env=vulkan1.3 -g"

set "_compiled=%~dp0/Compiled"

if NOT EXIST "%_compiled%" (
	md "%_compiled%"
)

if "%~dp1" EQU "" ( exit /B 1 )

set "_vertShader=%~dpn1.vert"
set "_fragShader=%~dpn1.frag"

if EXIST "%_vertShader%" (
	echo Compiling vert shader "%_vertShader%"
	%_glslc% %_vertShader% -o "%_compiled%/%~n1_vert.spv"
)

if EXIST "%_fragShader%" (
	echo Compiling frag shader "%_fragShader%"
	%_glslc% %_fragShader% -o "%_compiled%/%~n1_frag.spv"
)

pause