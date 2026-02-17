@echo off

:: Ensure the current working directory is the batch script's directory
cd /d "%~dp0"

:: Check for msbuild in PATH
echo Checking if msbuild is available...
msbuild /version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: msbuild is not installed or not in PATH.
    echo Please install msbuild or ensure it is correctly added to your PATH environment variable.
    exit /b 1
) else (
    echo msbuild is available.
)

:: Check for vcpkg in PATH
echo Checking if vcpkg is available...
vcpkg version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: vcpkg is not installed or not in PATH.
    echo Please install vcpkg or ensure it is correctly added to your PATH environment variable.
    exit /b 1
) else (
    echo vcpkg is available.
)

git submodule status
:: Check if any submodules are NOT initialized (prefix is -)
git submodule status | findstr /R "^-" >nul
if %errorlevel% equ 0 (
    echo.
    echo WARNING: Some submodules are not initialized.
    echo Running 'git submodule update --init --recursive'
    git submodule update --init --recursive
) else (
    echo All submodules are initialized.
)

echo Check if nuget.exe exists
if not exist "nuget.exe" (
    echo nuget.exe not found. Downloading...
    curl -L https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -o nuget.exe
) else (
    echo nuget.exe already exists. Skipping download.
)

echo NuGet Restore
.\nuget.exe restore covlcov.slnx

echo Setting up vcpkg dependencies
cd OpenCppCoverage
vcpkg install
vcpkg integrate install
cd ..
cd lcov
vcpkg install
vcpkg integrate install
cd ../lcovTest
vcpkg install
vcpkg integrate install
cd ..

echo Building Solution x64
msbuild /m ./OpenCppCoverage/CppCoverage.slnx /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v143
msbuild /m ./OpenCppCoverage/CppCoverage.slnx /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
msbuild /m covlcov.slnx /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v143
msbuild /m covlcov.slnx /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143

echo Bootstrap completed successfully!
exit /b 0

