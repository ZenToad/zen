@echo off
setlocal enableextensions enabledelayedexpansion

set target=all_tests
set target_exe=%target%
set compiler=cl
set configuration=debug
set target_src=
set target_tests=..\tests\test_static_array.c
set target_include=-I..\..\
set target_lib=

if not exist bin mkdir bin

for %%a in (%*) do (
	set target=%%a
)

set target_c=..\tests\%target%.c

echo -------------------------------------------------
echo - Compiler:      %compiler%
echo - Configuration: %configuration%
echo - Target C:      %target%
echo - Source C:      %target_src%
echo - Includes:      %target_include%
echo - Output:        bin\%target%.exe
echo -------------------------------------------------
rem MT for statically linked CRT, MD for dynamically linked CRT
set win_runtime_lib=MT
set common_c=!target_c! !target_tests! !target_src! /Fe!target!.exe -nologo -TC -FC -EHa- !target_include!
set common_l=/SUBSYSTEM:CONSOLE /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRTD !target_lib!

echo.
echo Compiling...
pushd bin
	cl !common_c! -!win_runtime_lib!d -Od -Z7 /link !common_l!
    if "%ERRORLEVEL%" EQU "0" (
        goto good
    )
    if "%ERRORLEVEL%" NEQ "0" (
        goto bad
    )
:good
    !target!.exe
    goto done
:bad
    goto done
:done

popd

:end
