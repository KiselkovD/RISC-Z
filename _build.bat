@echo off
REM Удаляем папку build целиком (если существует)
rmdir /s /q build

REM Создаём папку build\debug и генерируем Debug конфигурацию
cmake -S . -B build\debug -DCMAKE_BUILD_TYPE=DEBUG
if errorlevel 1 (
    echo Error: Failed to generate Debug build configuration
    exit /b 1
)

REM Создаём папку build\release и генерируем Release конфигурацию
cmake -S . -B build\release -DCMAKE_BUILD_TYPE=RELEASE
if errorlevel 1 (
    echo Error: Failed to generate Release build configuration
    exit /b 1
)

echo Build configurations generated successfully.
pause
