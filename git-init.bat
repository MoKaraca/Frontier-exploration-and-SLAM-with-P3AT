@echo off
REM Git Repository Initialization Script for ROS Workspace
REM Batch file version for Windows Command Prompt
REM Usage: git-init.bat

setlocal enabledelayedexpansion

echo.
echo ==========================================
echo ROS Catkin Workspace Git Setup
echo ==========================================
echo.

REM Check if git is installed
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo [X] Git is not installed. Please install it first:
    echo     Download from: https://git-scm.com/download/win
    echo.
    echo     During installation, select:
    echo     - Use Git from Command Prompt
    echo     - Checkout as-is, commit as LF
    echo.
    pause
    exit /b 1
)

for /f "tokens=*" %%i in ('git --version') do set GIT_VERSION=%%i
echo [OK] Git is installed: %GIT_VERSION%
echo.

REM Get script directory
set SCRIPT_DIR=%~dp0

REM Remove trailing backslash for cleaner display
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

REM Check if already a git repository
if exist "%SCRIPT_DIR%\.git" (
    echo [WARNING] This directory is already a git repository!
    echo Location: %SCRIPT_DIR%
    echo.
    echo Current status:
    cd /d "%SCRIPT_DIR%"
    git status
    pause
    exit /b 0
)

echo Checking git configuration...

REM Check git name
for /f "tokens=*" %%i in ('git config --global user.name 2^>nul') do set GIT_NAME=%%i

if "!GIT_NAME!"=="" (
    echo [WARNING] Git user name not configured.
    set /p GIT_NAME="Enter your name: "
    call git config --global user.name "!GIT_NAME!"
)

REM Check git email
for /f "tokens=*" %%i in ('git config --global user.email 2^>nul') do set GIT_EMAIL=%%i

if "!GIT_EMAIL!"=="" (
    echo [WARNING] Git email not configured.
    set /p GIT_EMAIL="Enter your email: "
    call git config --global user.email "!GIT_EMAIL!"
)

echo [OK] Git configured as: !GIT_NAME! ^<!GIT_EMAIL!^>
echo.

REM Initialize repository
echo Initializing git repository...
cd /d "%SCRIPT_DIR%"
git init
echo [OK] Repository initialized
echo.

REM Add all files
echo Adding files to staging area...
git add .
echo [OK] Files staged
echo.

REM Show status
echo Files to be committed:
git status --short
echo.

REM Create initial commit
echo Creating initial commit...
git commit -m "Initial commit: ROS catkin workspace with frontier exploration and detection nodes"
echo [OK] Initial commit created
echo.

REM Show final status
echo ==========================================
echo [OK] Repository successfully initialized!
echo ==========================================
echo.

echo Repository location: %SCRIPT_DIR%
echo Commit history:
git log --oneline -n 5
echo.

REM Optional: Set up remote
set /p SETUP_REMOTE="Would you like to connect to a remote repository? (y/n): "

if /i "%SETUP_REMOTE%"=="y" (
    echo.
    echo Enter remote repository URL (e.g., https://github.com/username/repo.git):
    set /p REMOTE_URL=
    
    if not "!REMOTE_URL!"=="" (
        git remote add origin "!REMOTE_URL!"
        echo [OK] Remote added: origin
        echo.
        echo To push your code, run:
        echo   git branch -M main
        echo   git push -u origin main
    )
)

echo.
echo Next steps:
echo 1. Make changes to your code
echo 2. Stage changes: git add .
echo 3. Commit changes: git commit -m "Your message"
echo 4. Push to remote: git push
echo.
echo Learn more: Read GIT_SETUP.md in the workspace root
echo.

pause
