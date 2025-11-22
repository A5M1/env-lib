@echo off
call e:\buildtools\devcmd.bat

set ENV_DIR=env
set TEST_DIR=test
set OBJ_DIR=obj
set BIN_DIR=bin
set ENV_LIB=%BIN_DIR%\env.lib
set TARGET=%BIN_DIR%\test.exe

if not exist %OBJ_DIR% mkdir %OBJ_DIR%
if not exist %BIN_DIR% mkdir %BIN_DIR%

for %%f in (%ENV_DIR%\*.c) do cl /c /nologo /W3 /O2 /I %ENV_DIR% %%f /Fo%OBJ_DIR%\%%~nf.obj

lib /nologo /OUT:%ENV_LIB% %OBJ_DIR%\*.obj

for %%f in (%TEST_DIR%\*.c) do cl /c /nologo /W3 /O2 /I %ENV_DIR% /I %TEST_DIR% %%f /Fo%OBJ_DIR%\%%~nf.obj

cl /nologo /Fe%TARGET% %OBJ_DIR%\*.obj %ENV_LIB%

:: --- create .env file in output directory ---
(
echo API_KEY=foo
echo USER=foo
) > %BIN_DIR%\.env

echo Generating documentation...
doxygen Doxyfile

echo Build complete.
pause
