@echo off
REM Проверяем, что передан путь к бинарнику
if "%~1"=="" (
    echo Usage: %~nx0 path_to_binary
    exit /b 1
)

REM По умолчанию запускаем Debug сборку, можно изменить путь ниже
set EMU_PATH=build\debug\risc-z.exe

REM Проверяем, что исполняемый файл существует
if not exist "%EMU_PATH%" (
    echo Emulator executable not found: %EMU_PATH%
    exit /b 1
)

REM Запускаем эмулятор с указанным бинарником
"%EMU_PATH%" "%~1"

pause
